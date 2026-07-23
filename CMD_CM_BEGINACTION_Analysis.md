# Technical Investigation: Client-Side and Server-Side Mechanics of `CMD_CM_BEGINACTION`

This document details the reverse-engineered mechanics of **`CMD_CM_BEGINACTION`** (Packet ID `6`, or `CMD_CM_BASE + 6`) in the Pirate King Online (PKO) protocol, specifically analyzing the structures, server-side validation rules, and proper implementation strategies for:
1. **`enumACTION_SKILL`** (Action ID `2`) — Including the critical `chMove` configuration and path sequence serialization.
2. **`enumACTION_ITEM_PICK`** (Action ID `8`) — Including the server-side distance constraint (`defPICKUP_DISTANCE`) and proximity handling.

Additionally, this document contains an in-depth investigation of stateful coordinate tracking for players, monsters, and items, detailing the exact wire layouts for:
3. **`CMD_MC_CHABEGINSEE`** (Packet ID `504`) — For obtaining initial spawn and teleportation coordinates.
4. **`CMD_MC_NOTIACTION`** (Packet ID `75`) — For monitoring real-time coordinate updates of moving entities.

---

## 1. Overview of `CMD_CM_BEGINACTION`

`CMD_CM_BEGINACTION` is a foundational client-to-server (C->S) packet that signals the start of player-initiated actions. The packet header consists of the generic binary header, followed by the active player character's context and the action type.

### Binary Layout of the Packet Header
When sent over the wire, a decrypted `CMD_CM_BEGINACTION` packet has the following common prefix layout:

| Byte Range | Data Type | Field Name | Description |
|---|---|---|---|
| `0 - 1` | `uint16` (Big-Endian) | `Size` | Total length of the packet in bytes. |
| `2 - 5` | `uint32` (Big-Endian) | `Session` | Connection session identifier. |
| `6 - 7` | `uint16` (Big-Endian) | `Command` | Packet ID, always `6` (`CMD_CM_BEGINACTION`). |
| `8 - 11` | `uint32` (Big-Endian/Little-Endian) | `PacketCount` | Monotonically increasing sequence count (handled by ProxySession). |
| `12 - 15` | `uint32` (Little-Endian) | `PlayerWorldId` | Character's world instance ID (`ulWorldID`). |
| `16` | `uint8` (Byte) | `ActionType` | The action being performed (e.g., `2` for skill, `8` for picking items). |

---

## 2. Analysis of `enumACTION_SKILL` (Action ID `2`)

### 2.1 Packet Layout & Wire Serialization
The behavior of physical attacks (basic attacks) and magical skill executions is determined by the `chMove` parameter of the skill action structure.

#### The C++ Client-Side Structure (`stNetSkillInfo`)
```cpp
struct stNetSkillInfo // enumACTION_SKILL
{
    BYTE byFightID;
    char chMove; // 1: Direct use skill (Melee/Instant). 2: Move to position first, then use skill.

    long lSkillID; // 0: Basic physical attack. >0: Skill ID.

    // Target or position coordinates
    union {
        struct {
            long lTarInfo1; // Target Entity World ID (if target is entity)
            long lTarInfo2; // Target Entity Handle (if target is entity)
        };
        struct {
            long lPosX;     // Target X (if target is ground position)
            long lPosY     // Target Y (if target is ground position)
        };
    };

    stNetMoveInfo SMove; // Contains the pre-movement path details if chMove == 2
};
```

#### Wire Serialization Sequence (`ProCirculateCS.cpp`):
```cpp
case enumACTION_SKILL: {
    stNetSkillInfo* pSkill = (stNetSkillInfo*)param;
    pk.WriteChar(pSkill->chMove);
    pk.WriteChar(pSkill->byFightID);
    if (pSkill->chMove == 2) {
        pk.WriteSequence((cChar*)pSkill->SMove.pos_buf, uShort(sizeof(POINT) * pSkill->SMove.pos_num));
    }
    pk.WriteLong(pSkill->lSkillID);
    pk.WriteLong(pSkill->lTarInfo1);
    pk.WriteLong(pSkill->lTarInfo2);
    pCNetIf->SendPacketMessage(pk);
    break;
}
```

#### Field layout after byte `16` (`ActionType = 2`):

##### Case A: `chMove = 1` (Direct / Direct Attack) — **DEPRECATED/DISABLED BY SERVER**
*Note: Although documented, this layout is completely rejected by modern GameServers.*
- `17`: `chMove = 1`
- `18`: `byFightID`
- `19 - 22`: `lSkillID`
- `23 - 26`: `lTarInfo1` (Target ID or X-coordinate)
- `27 - 30`: `lTarInfo2` (Target Handle or Y-coordinate)

##### Case B: `chMove = 2` (Move-to-Target, then Execute Skill) — **MANDATORY FOR ALL ATTACKS**
- `17`: `chMove = 2`
- `18`: `byFightID`
- `19 - 20`: `PathSequenceLength` (`uint16`, Big-Endian length of the path bytes, i.e., `pos_num * 8`)
- `21 - ...`: `PathPoints` (Raw sequence of `POINT` structures, where each `POINT` is 8 bytes: 4-byte `x` and 4-byte `y`)
- `Next 4 bytes`: `lSkillID` (4-byte signed integer, `0` for basic attack)
- `Next 4 bytes`: `lTarInfo1` (Target Entity World ID)
- `Next 4 bytes`: `lTarInfo2` (Target Handle / `0`)

---

### 2.2 Critical Server-Side Discovery
When a player character invokes a skill action, the server parses the `chMove` byte under `CCharacter::BeginAction(RPACKET pk)` inside `CharacterPrl.cpp`:

```cpp
const char chMove = READ_CHAR(pk);
if (chMove == 2) { // 移动到目标点后再使用技能
    const char chFightID = READ_CHAR(pk);
    Point Path[defMOVE_INFLEXION_NUM];
    Char chPointNum;
    uShort ulTurnNum;
    const char* pData = READ_SEQ(pk, ulTurnNum);

    // ... Path parsing & validation ...
    memcpy(Path, pData, chPointNum * sizeof(Point));

    const unsigned long ulSkillID = READ_LONG(pk);
    const long lTarInfo1 = READ_LONG(pk);
    const long lTarInfo2 = READ_LONG(pk);

    CSkillRecord* pRec = GetSkillRecordInfo(ulSkillID);
    // ...
    Cmd_BeginSkill((Short)m_dwPing, Path, chPointNum, pRec, 1, lTarInfo1, lTarInfo2);
} else {
    SystemNotice(RES_STRING(GM_CHARACTERPRL_CPP_00041));
    m_CLog.Log("the action type(directness use skills)has been cancellation[PacketID: %u]\n", ulPacketId);
    break;
}
```

#### Why the current Autobot physical attack implementation is broken:
The proxy bot currently attempts to attack nearby mobs using `chMove = 1` (direct physical attack):
```csharp
writer.WriteByte(2); // enumACTION_SKILL
writer.WriteByte(1); // chMove (direct physical attack)
writer.WriteByte(0); // byFightID
writer.WriteUint32(0); // lSkillID
writer.WriteUint32(attackMob.WorldId);
writer.WriteUint32(0);
```
Because `chMove == 1`, **the server instantly rejects the packet**, writing `the action type(directness use skills)has been cancellation` to the logs.

**Conclusion:** All physical attacks and skill executions must use `chMove = 2` and supply a valid movement path (even if it contains just a single point representing the character's current or target coordinates).

---

## 3. Analysis of `enumACTION_ITEM_PICK` (Action ID `8`)

### 3.1 Packet Layout & Wire Serialization
The item pick action is relatively straightforward, representing an instruction to pick up a specific item entity currently lying on the ground.

#### Wire Serialization Sequence (`ProCirculateCS.cpp`):
```cpp
case enumACTION_ITEM_PICK: // 捡道具
{
    stNetItemPick* pPick = (stNetItemPick*)param;
    pk.WriteLong(pPick->lWorldID);
    pk.WriteLong(pPick->lHandle);
    pCNetIf->SendPacketMessage(pk);
    break;
}
```

#### Layout after byte `16` (`ActionType = 8`):
- `17 - 20`: `lWorldID` (4-byte signed integer, World ID of the ground item)
- `21 - 24`: `lHandle` (4-byte signed integer, Instance Handle of the ground item)

---

### 3.2 Server-Side Distance Verification
When the server receives an item pick action, it validates the request inside `CCharacter::Cmd_PickupItem(uLong ulID, Long lHandle)` within `CharacterCmd.cpp`:

```cpp
if (!IsRangePoint(pCEnt->GetPos(), defPICKUP_DISTANCE)) {
    return enumITEMOPT_ERROR_DISTANCE;
}
```

The pickup distance constant is defined in `CompCommand.h`:
```cpp
#define defPICKUP_DISTANCE 350 // 捡取范围（厘米）- 3.5 meters
```

If the distance between the character's current position and the ground item's position exceeds **350 units (centimeters)**, the server rejects the pickup instruction and returns `enumITEMOPT_ERROR_DISTANCE` immediately.

#### Why the current Autobot looting implementation is broken:
The proxy bot currently listens to ground item appearances via `CMD_MC_ITEMBEGINSEE` (packet 506) and immediately sends `enumACTION_ITEM_PICK` to loot it, regardless of the character's physical distance from the item:
```csharp
writer.WriteByte(8); // enumACTION_ITEM_PICK
writer.WriteUint32(pickItem.WorldId);
writer.WriteUint32(pickItem.Handle);
```
If the item is farther away than 350cm, the server silent-drops or rejects the pick operation, causing the client to fail to collect the item.

---

## 4. Stateful Position Tracking (Server-to-Client Mechanics)

To operate safely and dynamically, a proxy/bot client must maintain 100% correct coordinate situational awareness. This requires parsing the initialization packet (`CMD_MC_CHABEGINSEE`) and real-time movement packets (`CMD_MC_NOTIACTION`).

### 4.1 Mob/Player Initial Coordinates via `CMD_MC_CHABEGINSEE` (Packet ID `504`)
When an entity (monster, NPC, player) spawns or teleports into the player's vicinity, the server sends `CMD_MC_CHABEGINSEE`. Initial coordinates are nested inside `WriteBaseInfo(pk)`.

#### Sequential Payload Structure of `CMD_MC_CHABEGINSEE`:
To reliably parse the coordinates, a sequential packet reader must progress through the dynamic and static fields of the packet payload (starting directly after the 8-byte generic binary header):

| Parsing Step | Data Type | Field Name / Action |
|---|---|---|
| 1 | `uint8` (Byte) | `chSeeType` (Skip/Read) |
| 2 | `uint32` (uint) | `ulCat` (Skip) |
| 3 | `uint32` (uint) | `ulWorldID` (**Track:** Entity Instance World ID) |
| 4 | `uint32` (uint) | `ulMainChaID` |
| 5 | `string` | `szMainChaName` (Utilizes PkoPacketReader.ReadString) |
| 6 | `uint8` (Byte) | `chGMLv` |
| 7 | `uint32` (uint) | `lHandle` |
| 8 | `uint8` (Byte) | `chCtrlType` (Skip) |
| 9 | `string` | `szName` (Read string, e.g., mob or player name) |
| 10 | `string` | `szMotto` (Read string) |
| 11 | `uint16` (ushort) | `sIcon` |
| 12 | `uint32` (uint) | `ulGuildID` |
| 13 | `string` | `szGuildName` (Read string) |
| 14 | `string` | `szGuildMotto` (Read string) |
| 15 | `string` | `szStallName` (Read string) |
| 16 | `uint16` (ushort) | `sExistState` |
| 17 | `uint32` (uint) | `lPosX` (**Track:** Mob/Player Initial X Coordinate) |
| 18 | `uint32` (uint) | `lPosY` (**Track:** Mob/Player Initial Y Coordinate) |

By implementing a sequential reader according to this pattern, the proxy can track both **mob positions** and the **player's own starting position** when entering maps or teleporting.

---

### 4.2 Dynamic Coordinate Updates via `CMD_MC_NOTIACTION` (Packet ID `75`)
Whenever an entity (including the player themselves) moves in the world, the server broadcasts `CMD_MC_NOTIACTION` with `ActionType == enumACTION_MOVE`.

#### Sequential Payload Structure of `CMD_MC_NOTIACTION`:
Starting immediately after the 8-byte generic binary header:

| Sequence | Data Type | Field Name | Description |
|---|---|---|---|
| 1 | `uint32` (uint) | `ulWorldID` | World ID of the moving character. |
| 2 | `uint32` (uint) | `ulPacketID` | Action packet ID. |
| 3 | `uint8` (Byte) | `chActionType` | Must be `1` (`enumACTION_MOVE`) for movement updates. |
| 4 | `uint16` (ushort) | `sState` | Movement state (e.g., `1` for ongoing movement). |
| 5 | `uint16` (ushort) | `sStopState` | **Conditional:** Only present if `sState != 1`. |
| 6 | `uint16` (Big-Endian) | `ulTurnNum` | Length of the subsequent path byte sequence (`pos_num * 8`). |
| 7 | `POINT[]` (Raw bytes) | `PathPoints` | Sequence of 8-byte structures (`uint32 x` and `uint32 y`) representing the coordinates of the path. |

#### Tracking Implementation Logic:
To keep coordinates of all surrounding entities up-to-date in real-time, the proxy/bot should:
1. Intercept `CMD_MC_NOTIACTION` (Packet ID `75`).
2. Verify that `chActionType == 1` (`enumACTION_MOVE`).
3. Read the array of `PathPoints`. The last point in the sequence represents the final target destination coordinate:
   ```csharp
   int numPoints = ulTurnNum / 8;
   if (numPoints > 0) {
       // Seek to the last point of the path
       int lastPointOffset = (numPoints - 1) * 8;
       uint finalX = reader.ReadUint32At(lastPointOffset);
       uint finalY = reader.ReadUint32At(lastPointOffset + 4);

       // Update tracked entity coordinate state
       UpdateEntityCoordinates(ulWorldID, finalX, finalY);
   }
   ```
4. If `ulWorldID == PlayerWorldId`, update the bot's own internal `(playerX, playerY)` tracked positions!

---

## 5. Remediation Blueprint & Proper Client Strategy

To resolve both issues in an automated bot or proxy sequence injector, the client/proxy must dynamically calculate distances and sequence movement paths before performing actions.

### 5.1 Melee Attack & Skill Remediation
To successfully execute a basic attack or a skill, the injector must write `chMove = 2` and supply a coordinate path.

#### Fixed Attack Payload Structure:
```csharp
var writer = new PkoPacketWriter();
writer.WriteUint16(0);                  // size placeholder
writer.WriteUint32(targetSessionId);
writer.WriteUint16(6);                  // CMD_CM_BEGINACTION
writer.WriteUint32(0);                  // sequence placeholder
writer.WriteUint32(targetPlayerId);
writer.WriteByte(2);                    // enumACTION_SKILL
writer.WriteByte(2);                    // chMove = 2 (REQUIRED)
writer.WriteByte(0);                    // byFightID

// Construct Path Sequence (Length + POINT nodes)
// For a simple standing-still attack or short lunge, we can write a single path point (the player's current X, Y)
ushort pathBytesLength = 8;             // 1 point * sizeof(POINT)
writer.WriteUint16(pathBytesLength);    // PathSequenceLength (Big-Endian!)
writer.WriteUint32((uint)playerX);      // POINT.x
writer.WriteUint32((uint)playerY);      // POINT.y

writer.WriteUint32(0);                  // lSkillID (0 = physical attack)
writer.WriteUint32(attackMob.WorldId);   // lTarInfo1
writer.WriteUint32(0);                  // lTarInfo2 (Handle)
```

---

### 5.2 Safe Looting Remediation (Move-to-Item First)
Ground items do not support a `chMove` parameter inside `enumACTION_ITEM_PICK`. Therefore, to safely loot an item:
1. The proxy must keep track of the player's current coordinates `(playerX, playerY)`.
2. When an item is selected for looting:
   - Calculate the Euclidean distance:
     $$\text{Distance} = \sqrt{(\text{itemX} - \text{playerX})^2 + (\text{itemY} - \text{playerY})^2}$$
   - If $\text{Distance} \le 350$, directly inject `enumACTION_ITEM_PICK` (Action ID `8`).
   - If $\text{Distance} > 350$, first inject `enumACTION_MOVE` (Action ID `1`) with a path leading to `(itemX, itemY)`.
   - Wait for the player to reach the destination (or periodically monitor coordinate updates), and then inject `enumACTION_ITEM_PICK` once the distance check passes.

#### Sample Safe Looting Flow:

```
        +----------------------------------------+
        |        Ground Item Detected            |
        +----------------------------------------+
                            |
                            v
        +----------------------------------------+
        |  Calculate distance to item (dx, dy)   |
        +----------------------------------------+
                            |
             Is Distance <= 350?
             /                  \
           YES                   NO
           /                       \
          v                         v
+------------------------+  +------------------------------------+
|  Inject ACTION_PICK    |  |  Inject ACTION_MOVE path to item   |
|  (Action ID 8)         |  |  (Action ID 1)                     |
+------------------------+  +------------------------------------+
                                            |
                                            v
                            +------------------------------------+
                            |  Monitor coordinate updates        |
                            |  Wait until Distance <= 350        |
                            +------------------------------------+
                                            |
                                            v
                            +------------------------------------+
                            |  Inject ACTION_PICK                |
                            |  (Action ID 8)                     |
                            +------------------------------------+
```
