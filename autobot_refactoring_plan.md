# Pirate King Online (PKO) - AutoBot Plugin Deep-Dive Investigation & Implementation Plan

This document provides a comprehensive protocol investigation and a detailed, reference-based engineering plan to refactor and optimize the `AutoBotPlugin` in `PkoProxyClient`. It addresses existing runtime bugs—including coordinate drift, illegal movement requests, and non-functional targeting/attacks—by tracing protocol structures back to their origins in the Pirate King Online (PKO) GameServer/Client C++ source code.

---

## 1. Executive Summary

The current C# implementation of the `AutoBotPlugin` fails to perform coordinated movement, attacking, or looting because of structural protocol misunderstandings, sequence number misalignments, and state tracking deficiencies.

### Core Problems Identified:
1. **Sequence Offset Corruption**: For `CMD_CM_BEGINACTION` (Command 6), C#'s sequence tracking centrally overwrites bytes 8–11 of the packet payload. However, in this specific packet, bytes 8–11 contain the player's Character Attach ID (`ulWorldID`), whereas the actual `packetCount` sequence number sits at bytes 12–15. This corruption invalidates the character ID, leading to server-side action rejection or immediate socket drops.
2. **Action Overlap / State Blockage**: Injected bot actions (movement, attacks, looting) are frequently sent while a previous action is still active on the server. The server rejects these with a "foregone action hasn't finished" warning (`FailedActionNoti` with reason `enumFACTION_EXISTACT`). There is no mechanism to end or interrupt previous actions.
3. **Severe Coordinate Drift**: The plugin only captures coordinates on initial entity spawns (`CMD_MC_CHABEGINSEE`, Packet 504) or on client-initiated move packets. It completely ignores real-time movement updates broadcast by the server via `CMD_MC_NOTIACTION` (Packet 508), making distance checks for looting and attacking highly inaccurate.
4. **Inefficient Looting**: Ground items are picked up one by one per loop cycle. PKO natively supports mass looting via `enumACTION_TOTAL_ITEM_PICK` (Action 54), which is entirely unused.

---

## 2. Deep-Dive Root Cause Analysis

### Issue A: "Illegal movement request / previous movement has not ended"

#### Server-Side Source Reference: `CharacterPrl.cpp`
When a player issues a move or skill command, the request is parsed by `CCharacter::BeginAction` in `CharacterPrl.cpp`:
```cpp
// source/src/GameServer/CharacterPrl.cpp

void CCharacter::BeginAction(RPACKET pk) {
    ...
    switch (chActionType) {
    case enumACTION_MOVE: {
        ...
        if (m_CAction.GetCurActionNo() >= 0) { // Previous action has not ended
            FailedActionNoti(enumACTION_MOVE, enumFACTION_EXISTACT);
            SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00035)); // "foregone action hasn't finished"
            m_CLog.Log("irregular action request(foregone action hasn't finish)[PacketID: %u]\n", ulPacketId);
            break;
        }
        ...
    }
    case enumACTION_SKILL: {
        ...
        if (m_CAction.GetCurActionNo() >= 0) { // Previous action has not ended
            FailedActionNoti(enumACTION_SKILL, enumFACTION_EXISTACT);
            SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00035));
            m_CLog.Log("irregular action request(foregone action hasn't finish)[PacketID: %u]\n", ulPacketId);
            break;
        }
        ...
    }
```
If `m_CAction.GetCurActionNo() >= 0`, the player character is currently executing an action (e.g., walking along a path, cast animation, looting). The server enforces a strict rule: **a client cannot start a new action without finishing or canceling the current one**.

*   **Why the bot fails**: The bot loops every 1500ms and blindly injects movement or combat packets. If the player is still traveling along a path or executing an action, the server rejects the injection with `enumFACTION_EXISTACT`.
*   **The Client-Side Solution**: In `ProCirculateCS.cpp` (C++ client), when a movement is interrupted, or a new target is clicked, the client first sends `CMD_CM_ENDACTION` (Command 7) to clear the active action sequence:
    ```cpp
    void CProCirculateCS::EndAction(CActionState* pState) {
        WPacket pk = pCNetIf->GetWPacket();
        pk.WriteCmd(CMD_CM_ENDACTION); // 7
        pCNetIf->SendPacketMessage(pk);
    }
    ```
    This calls `CCharacter::EndAction(RPACKET pk)` on the server, executing `m_CAction.End()` which sets the current action number back to `-1`, resetting the state machine and clearing the path.

---

### Issue B: "Targeting/attacking doesn't move" & "Moving doesn't move"

The underlying issue is a severe byte-alignment and sequence number corruption within the `ProxySession.SendClientPacketAsync` central pipeline and the `AutoBotPlugin` packet construction.

#### The Byte Offset Misalignment:
In `PkoPacket.cs`, the properties are mapped as:
```csharp
public bool HasPacketCount => RawBytes.Length >= 12;

public uint PacketCount
{
    get => (uint)((RawBytes[8] << 24) | (RawBytes[9] << 16) | (RawBytes[10] << 8) | RawBytes[11]);
    set { ... }
}
```
This assumes that **any** packet longer than 12 bytes starts its payload with a 4-byte `packetCount` sequence number at index `8`.
However, let's examine the exact serialization logic for `CMD_CM_BEGINACTION` (6) in `ProCirculateCS.cpp`:
```cpp
void CProCirculateCS::BeginAction(CCharacter* pCha, DWORD type, const void* param, CActionState* pState) {
	WPacket pk = pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_BEGINACTION);              // Opcode = 6 (2 bytes, indices 6-7)
	pk.WriteLong(pCha->getAttachID());            // Player World ID (4 bytes, indices 8-11)

#ifdef defPROTOCOL_HAVE_PACKETID
	pk.WriteLong(pCNetIf->m_ulPacketCount++);     // Sequence number / packetCount (4 bytes, indices 12-15)
#endif
	pk.WriteChar((dbc::uChar)type);               // Action Type (1 byte, index 16)
```

#### The Corruption Walkthrough:
1. When `ProxySession` receives or injects `CMD_CM_BEGINACTION`, `RawBytes.Length` is greater than 12, so `HasPacketCount` is true.
2. `ProxySession` reads `pkt.PacketCount` (reading bytes 8–11) and overwrites bytes 8–11 with the incremented rolling sequence number (`NextPacketCount++`).
3. But bytes 8–11 actually contain the **Player's Character Attach ID / World ID**!
4. Consequently:
    - The player's Character Attach ID is corrupted and replaced by a sequence number (e.g. `1`, `2`, `3`).
    - The actual `packetCount` at bytes 12–15 remains untouched or carries incorrect values.
    - The GateServer and GameServer receive a packet with an invalid character ID and reject it instantly as unauthorized, meaning the bot cannot move or attack.

#### The Shifted Payload in Bot Injection:
To make matters worse, the current `AutoBotPlugin.cs` attempts to inject movement like this:
```csharp
var writer = new PkoPacketWriter();
writer.WriteUint16(0);                     // Bytes 0-1: Size
writer.WriteUint32(targetSessionId);       // Bytes 2-5: Session
writer.WriteUint16(6);                     // Bytes 6-7: Command ID (6)
writer.WriteUint32(0);                     // Bytes 8-11: Sequence placeholder (Gets overwritten by ProxySession)
writer.WriteUint32(targetPlayerId);        // Bytes 12-15: Written as player ID
writer.WriteUint32(_packetID++);           // Bytes 16-19: Written as dummy packet ID
writer.WriteByte(1);                       // Byte 20: Action Type (1 = Move)
```
This layout is completely shifted:
- The server expects the action type at byte index **16**. But the bot writes `_packetID++` (dummy packet ID) at bytes 16–19, shifting the Action Type to byte **20**, and the movement path to byte **21**.
- The server decodes junk bytes, causing immediate packet-validation failures.

---

### Issue C: "Picking up only works if adjacent / coordinates drift"

Looting has a range enforcement check against `defPICKUP_DISTANCE` (defined as 350 units/cm in `CompCommand.h`). The server performs this range check using the player's actual coordinate state on the map.

```cpp
// source/src/GameServer/CharacterCmd.cpp
if (!IsRangePoint(pCEnt->GetPos(), defPICKUP_DISTANCE)) {
    return enumITEMOPT_ERROR_DISTANCE; // 350 units
}
```

The current C# plugin maintains local coordinates `_playerX` and `_playerY`, but updates them only when:
1. `CMD_CM_BEGINACTION` (6) C->S is caught.
2. `CMD_MC_CHABEGINSEE` (504) S->C is caught (spawning).

#### The Drift Problem:
When the character moves, is pushed, hits obstacles, or walks along a path, the client interpolates and the server continuously updates the coordinates, broadcasting them back to the client via `CMD_MC_NOTIACTION` (508) packets. Because the bot completely ignores `CMD_MC_NOTIACTION`, its local `_playerX` and `_playerY` get out of sync with the server.
- The bot *believes* it is adjacent to the item (distance <= 350), so it sends a loot packet directly.
- The server calculates the distance using its own (correct) coordinates, realizes the player is actually far away, and rejects the loot request with `enumITEMOPT_ERROR_DISTANCE`.

---

## 3. Reference-Based Protocol Specifications

To successfully refactor the bot, we must align all injected and parsed packets to the exact memory-structures defined in the C++ sources.

### A. `CMD_CM_BEGINACTION` (Command 6) [C -> S]
Used for moving, attacking, casting skills, and picking up items.

| Byte Range | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| **0 - 1** | `ushort` | Size | Total packet size (big-endian) |
| **2 - 5** | `uint` | Session ID | Connection Session ID |
| **6 - 7** | `ushort` | Command | `CMD_CM_BEGINACTION` = `6` |
| **8 - 11** | `uint` | `ulWorldID` | Active Character Attach ID / Player World ID |
| **12 - 15** | `uint` | `packetCount` | Dynamic rolling sequence number |
| **16** | `byte` | `chActionType` | Action Enum Type (e.g. `1` = Move, `2` = Skill, `8` = Loot, `54` = TotalPick) |

#### Action-Specific Payloads (Starting at Byte 17):

*   **`enumACTION_MOVE` (1)**:
    *   **Bytes 17 - 18 (ushort)**: Number of path bytes (calculated as `8 * PointCount`).
    *   **Bytes 19 - End (Point[])**: Sequential array of 8-byte `Point` structures (`long x`, `long y`).

*   **`enumACTION_SKILL` (2)**:
    *   **Byte 17 (byte)**: `chMove` (`2` = Pre-movement path is attached, `1` = Direct casting - disabled).
    *   **Byte 18 (byte)**: `byFightID` (Combat status ID, typically `64` / `0x40`).
    *   **Bytes 19 - 20 (ushort)**: Only present if `chMove == 2`. Specifies pre-movement path length in bytes (`8 * PointCount`).
    *   **Bytes 21 - ... (Point[])**: Only present if `chMove == 2`. Pre-movement path points.
    *   **Next 4 Bytes (uint)**: `ulSkillID` (Skill ID to use; basic attack is usually `1`).
    *   **Next 4 Bytes (uint)**: Target World ID (or target X coordinate).
    *   **Next 4 Bytes (uint)**: Target Handle (or target Y coordinate).

*   **`enumACTION_ITEM_PICK` (8)**:
    *   **Next 4 Bytes (uint)**: Item World ID (`ulID`).
    *   **Next 4 Bytes (uint)**: Item Handle (`lHandle`).

*   **`enumACTION_TOTAL_ITEM_PICK` (54)**:
    *   **Bytes 17 - 18 (short)**: `nCount` (Number of items to pick up, capped between `1` and `48`).
    *   **Loop of `nCount` elements**:
        *   **4 Bytes (uint)**: Item World ID (`ulID`).
        *   **4 Bytes (uint)**: Item Handle (`lHandle`).

---

### B. `CMD_CM_ENDACTION` (Command 7) [C -> S]
Terminates or cancels the current action state, resetting the action number to `-1`.

| Byte Range | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| **0 - 1** | `ushort` | Size | Total packet size (8 bytes) |
| **2 - 5** | `uint` | Session ID | Connection Session ID |
| **6 - 7** | `ushort` | Command | `CMD_CM_ENDACTION` = `7` |

---

### C. `CMD_MC_NOTIACTION` (Command 508) [S -> C]
Broadcasts movement and combat state transitions in real-time.

| Byte Range | Type | Field | Description |
| :--- | :--- | :--- | :--- |
| **0 - 1** | `ushort` | Size | Total packet size |
| **2 - 5** | `uint` | Session ID | Session ID |
| **6 - 7** | `ushort` | Command | `CMD_MC_NOTIACTION` = `508` |
| **8 - 11** | `uint` | Entity World ID | ID of character/mob moving/acting |
| **12 - 15** | `uint` | Packet ID | Sequence identifier |
| **16** | `byte` | Action Type | Typically `enumACTION_MOVE` = `1` |
| **17 - 18** | `ushort` | `sState` | Movement status (e.g., `0` = ON, `1` = ARRIVE, `2` = BLOCK) |
| **19 - 20** | `ushort` | `sStopState` | Only present if `sState != enumMSTATE_ON` (0) |
| **Next 2 Bytes** | `ushort` | Sequence Bytes | Length of path bytes (`8 * PointCount`) |
| **Next Bytes** | `Point[]` | Path Points | Array of `Point` coordinates (each 8 bytes: `long x`, `long y`) |

---

## 4. AutoBot Plugin Refactoring & Improvement Architecture

To fix the bot cleanly and introduce advanced features without code regression, we design a stateful, modular, and robust architecture.

```
+--------------------------------------------------------------------------+
|                             ProxySession                                 |
+-----------------------------------++-------------------------------------+
                                    ||
                 Forward Decrypted  ||  Intercept & Read Coordinates
                     C->S Packets   \/
+--------------------------------------------------------------------------+
|                             AutoBotPlugin                                |
|                                                                          |
|  +--------------------+   +-----------------------+   +---------------+  |
|  | Real-Time Coords   |   | Stateful Bot Engine   |   | Target Filter |  |
|  |  Tracking System   |   |   & State Machine     |   |   & Selector  |  |
|  +---------+----------+   +-----------+-----------+   +-------+-------+  |
|            |                          |                       |          |
|            | Read Actual              | Transition            | Evaluate |
|            v                          v                       v          |
|  +--------------------------------------------------------------------+  |
|  |                        Action Controller                           |  |
|  |  - Prevents overlap using CMD_CM_ENDACTION                         |  |
|  |  - Generates perfectly aligned Move, Skill, and Loot Packets        |  |
|  +--------------------------------------------------------------------+  |
+--------------------------------------------------------------------------+
```

### A. Stateful Action Controller & State Machine
The bot needs an explicit state machine to prevent action overlapping:
```csharp
public enum BotState
{
    Idle,
    MovingToMob,
    Attacking,
    Looting,
    Patrolling
}
```

#### Core Logic:
- **Action Cancellation**: Before transitioning from `BotState.MovingToMob` to `BotState.Looting`, or before switching movement vectors, the bot must call `SendEndActionPacket()`.
- **State Lock**: While an action is in progress, the state is locked. The bot will not spam packets on subsequent timer loops. It will wait for the character to either:
  1. Receive `CMD_MC_NOTIACTION` (508) indicating the path has finished (state `enumMSTATE_ARRIVE` or `enumMSTATE_BLOCK`).
  2. Experience an action timeout (safety trigger, e.g. 5 seconds of non-movement).

---

### B. Real-Time Position Sync & Tracking
The bot will listen to `CMD_MC_NOTIACTION` (508) packets to dynamically update player and mob positions in memory:

```csharp
if (context.Direction == "S -> C" && context.PacketId == 508)
{
    var reader = new PkoPacketReader(context.DecryptedPacket);
    _ = reader.ReadUint16(); // size
    _ = reader.ReadUint32(); // session
    _ = reader.ReadUint16(); // 508
    uint entityId = reader.ReadUint32();
    _ = reader.ReadUint32(); // packetId
    byte actionType = reader.ReadByte();

    if (actionType == 1) // Move action
    {
        ushort sState = reader.ReadUint16();
        if (sState != 0) // enumMSTATE_ON = 0. If stopped, stop-state is written
        {
            _ = reader.ReadUint16(); // sStopState
        }

        ushort pathNumBytes = reader.ReadUint16();
        if (pathNumBytes >= 8)
        {
            // Skip to the last point of the path
            reader.ReadBytes(pathNumBytes - 8);
            int finalX = (int)reader.ReadUint32();
            int finalY = (int)reader.ReadUint32();

            lock (_lock)
            {
                if (entityId == _playerWorldId)
                {
                    _playerX = finalX;
                    _playerY = finalY;
                }
                else if (_mobs.TryGetValue(entityId, out var mob))
                {
                    mob.X = finalX;
                    mob.Y = finalY;
                }
            }
        }
    }
}
```

This prevents coordinate drift entirely, assuring distance-checks for attacks or looting are always 100% accurate.

---

### C. Mass Looting Engine (`enumACTION_TOTAL_ITEM_PICK`)
Currently, picking up items takes multiple ticks because the bot loots a single item, waits, then loops.
To pick up all adjacent items instantaneously:
1. Scan the local `_items` registry.
2. Filter out items beyond the pickup range (350 units) or blacklisted items.
3. Batch the remaining items (up to 48) into a single C->S `enumACTION_TOTAL_ITEM_PICK` (54) packet.

#### Injected Payload Structure for Mass Looting:
```csharp
var writer = new PkoPacketWriter();
writer.WriteUint16(0);                     // Size placeholder
writer.WriteUint32(sessionId);             // Session
writer.WriteUint16(6);                     // CMD_CM_BEGINACTION
writer.WriteUint32(playerWorldId);         // Bytes 8-11: Player ID
writer.WriteUint32(0);                     // Bytes 12-15: Sequence Placeholder (re-written by ProxySession)
writer.WriteByte(54);                      // Byte 16: Action Type (54 = TotalPick)

writer.WriteUint16((ushort)itemsToLoot.Count); // Count
foreach (var item in itemsToLoot)
{
    writer.WriteUint32(item.WorldId);      // Item ID
    writer.WriteUint32(item.Handle);       // Item Handle
}
```

---

### D. Advanced Target Selection & Filtering

#### 1. Closest Mob Targeting:
Calculate Euclidean distance from player position to all available mobs in `_mobs` (excluding other players/characters):
$$\text{Distance} = \sqrt{(Mob_X - Player_X)^2 + (Mob_Y - Player_Y)^2}$$
Identify the closest mob. If beyond basic attack range, issue a movement path closing the distance, followed immediately by a pre-movement attack action (`chMove = 2`).

#### 2. Item & Mob Filters:
Introduce regular expression filters or string-matching lists in the bot configuration:
- **Item Whitelist/Blacklist**: Filters item names or item templates (`lID`) to pick up only valuable items (e.g., "Gold", "Gems", "Chest") and ignore trash items (e.g., "Rusty Sword").
- **Mob Target Filtering**: Prevents targeting specified high-level bosses, friendly NPCs, or players to prevent accidental deaths or detection.

---

### E. Patrolling Route Engine
When no enemies or loot are visible, the bot should navigate automatically to avoid standing still:
1. **Waypoint Grid / Patrol Route**: Define a pre-configured sequence of coordinates:
   `(X1, Y1) -> (X2, Y2) -> (X3, Y3) -> (X1, Y1)`
2. **Radial Search Vectors**: If no pre-configured path exists, the bot can generate circular searching paths centered around the starting coordinate (home anchor point) to probe and load new enemies.
3. When arriving at a patrol waypoint, check for newly spawned mobs/items. If found, exit patrol and initiate combat/looting.

---

## 5. Phase-by-Phase Implementation Plan

### Phase 1: Fix Protocol Corruptions
*   **Step 1.1**: Modify `PkoPacket.cs` to correctly align `PacketCount` for `CMD_CM_BEGINACTION`. Introduce a conditional check in `ProxySession.SendClientPacketAsync` or `PkoPacket`:
    - If the Command is `6` (`CMD_CM_BEGINACTION`), write the sequence number at **bytes 12-15** instead of **bytes 8-11**.
    - If the Command is anything else, fallback to standard offset (index 8).
*   **Step 1.2**: Fix `AutoBotPlugin` move and skill injection logic. Remove dummy offsets and write fields to the exact byte indices outlined in Section 3.

### Phase 2: Action Interruption & Coords Sync
*   **Step 2.1**: Implement a state machine (`BotState`) and coordinate `SendEndActionPacket()` helper inside `AutoBotPlugin`.
*   **Step 2.2**: Write the parsing logic for S->C `CMD_MC_NOTIACTION` (508) in `OnPacket` to constantly synchronize the player's coordinate variables.

### Phase 3: Advanced Combat & Looting Engine
*   **Step 3.1**: Implement `CalculateClosestMob()` using Euclidean distance. Automatically path toward the closest enemy and engage using skill injection with `chMove = 2`.
*   **Step 3.2**: Refactor item collection to compile adjacent items and execute a single mass-loot injection utilizing the `enumACTION_TOTAL_ITEM_PICK` (54) packet.

### Phase 4: Navigation & Patrol System
*   **Step 4.1**: Implement waypoint patrol routines.
*   **Step 4.2**: Incorporate filtering systems for items (Whitelist/Blacklist) and mobs.

---

This complete architecture addresses every core defect natively and cleanly within `PkoProxyClient`, turning the fragile, disconnected plugin into a highly effective, robust, and invisible automated gameplay assistant.
