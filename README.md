Product Annotation: WireGuard Service Manager

Introducing the WireGuard Service Manager, a user-friendly application designed to streamline your VPN experience. This tool offers seamless management of the WireGuard service, allowing users to effortlessly connect and disconnect from the VPN directly from the system tray.

Key Features:
1. WireGuard Service Management: Easily control the WireGuard service with a simple interface, ensuring optimal performance and security for your online activities.
2. Tray Application: Enjoy the convenience of connecting and disconnecting from your WireGuard VPN with just a click from the tray icon, providing quick access without interrupting your workflow.
3. Auto-Connect and Auto-Disconnect: Enhance your security with automatic connection and disconnection features. The application intelligently detects when a specified network adapter with a designated DNS zone is present, ensuring that your VPN is always active when needed and disconnected when not in use.
Experience the ultimate in VPN management with WireGuard VPN Manager, designed for users who value both security and convenience in their online presence.

After installing the product, you will need to make some changes to the Windows Registry in the "HKEY_LOCAL_MACHINE\SOFTWARE\Wireguard Service Manager" branch.
1. You must specify the name of the Wireguard service in the "WireGuardServiceName" registry value (e.g., "WireguardTunnel$Home").
2. You can set the DNS zone for the network adapter by specifying it in the "AutoConnectDNSZone" registry value. This will try to connect to VPN using this DNS zone (e.g., "home.local").
3. You can disable logging by setting the "EnableLog" parameter to 0 in the EventLog.
