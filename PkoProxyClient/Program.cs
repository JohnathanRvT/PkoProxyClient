using System;
using System.Threading;
using System.Threading.Tasks;

namespace PkoProxyClient
{
    /// <summary>
    /// Entry point class for the PKO C# Multi-Tool.
    /// Handles CLI commands for running crypt tests, a localized proxy server, or standalone headless client.
    /// </summary>
    class Program
    {
        static async Task Main(string[] args)
        {
            if (args.Length == 0)
            {
                PrintUsage();
                return;
            }

            string command = args[0].ToLower();

            if (command == "test")
            {
                bool success = PkoTest.RunTests();
                Environment.Exit(success ? 0 : 1);
            }
            else if (command == "proxy")
            {
                int localPort = 1973;
                string remoteHost = "135.125.152.11";
                int remotePort = 1973;
                bool protection = false;
                string logFile = "proxy_packets.log";
                string password = "";

                for (int i = 1; i < args.Length; i++)
                {
                    if (args[i] == "--local-port" && i + 1 < args.Length)
                        localPort = int.Parse(args[++i]);
                    else if (args[i] == "--remote-host" && i + 1 < args.Length)
                        remoteHost = args[++i];
                    else if (args[i] == "--remote-port" && i + 1 < args.Length)
                        remotePort = int.Parse(args[++i]);
                    else if (args[i] == "--protection")
                        protection = true;
                    else if (args[i] == "--log-file" && i + 1 < args.Length)
                        logFile = args[++i];
                    else if (args[i] == "--password" && i + 1 < args.Length)
                        password = args[++i];
                }

                var proxy = new PkoProxy(localPort, remoteHost, remotePort, protection, logFile, password);
                var cts = new CancellationTokenSource();
                Console.CancelKeyPress += (sender, eventArgs) =>
                {
                    eventArgs.Cancel = true;
                    cts.Cancel();
                };

                await proxy.StartAsync(cts.Token);
            }
            else if (command == "client")
            {
                string host = "135.125.152.11";
                int port = 1973;
                ushort version = 136;

                for (int i = 1; i < args.Length; i++)
                {
                    if (args[i] == "--host" && i + 1 < args.Length)
                        host = args[++i];
                    else if (args[i] == "--port" && i + 1 < args.Length)
                        port = int.Parse(args[++i]);
                    else if (args[i] == "--version" && i + 1 < args.Length)
                        version = ushort.Parse(args[++i]);
                }

                var client = new PkoClient(host, port, version);
                await client.RunAsync();
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"Unknown command: {command}");
                Console.ResetColor();
                PrintUsage();
            }
        }

        static void PrintUsage()
        {
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine(@"
===================================================
   ____  _  ______    __  __       _ _   _
  |  _ \| |/ / ___|  |  \/  |_   _| | |_(_)
  | |_) | ' / |  _   | |\/| | | | | | __| |
  |  __/| . \ |_| |  | |  | | |_| | | |_| |
  |_|   |_|\_\____|  |_|  |_|\__,_|_|\__|_|

        PKO C# Multi-Tool Console Panel
===================================================");
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("Usage:");
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("  dotnet run --project PkoProxyClient/PkoProxyClient.csproj -- <command> [arguments]");
            Console.WriteLine();
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("Commands:");
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("  test    - Run cryptography, encoders, and binary IO tests");
            Console.WriteLine("  proxy   - Run client-side proxy to inspect & dump unencrypted packets");
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine("    Arguments:");
            Console.WriteLine("      --local-port <port>    Local port to listen on (default: 1973)");
            Console.WriteLine("      --remote-host <host>   Target server IP/host (default: 135.125.152.11)");
            Console.WriteLine("      --remote-port <port>   Target server Port (default: 1973)");
            Console.WriteLine("      --protection           Enable sequence number protection parsing");
            Console.WriteLine("      --log-file <file>      Log file path for dumped packets (default: proxy_packets.log)");
            Console.WriteLine("      --password <password>  The account password to decrypt the session key correctly");
            Console.WriteLine();
            Console.ForegroundColor = ConsoleColor.White;
            Console.WriteLine("  client  - Connect as a playable client to the server");
            Console.ForegroundColor = ConsoleColor.DarkGray;
            Console.WriteLine("    Arguments:");
            Console.WriteLine("      --host <host>          Server IP/host to connect to (default: 135.125.152.11)");
            Console.WriteLine("      --port <port>          Server Port (default: 1973)");
            Console.WriteLine("      --version <version>    Client game version (default: 136)");
            Console.ForegroundColor = ConsoleColor.Magenta;
            Console.WriteLine("===================================================\n");
            Console.ResetColor();
        }
    }
}
