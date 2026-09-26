# systemd-if

`systemd-if` is a lightweight conditional wrapper for `systemctl` designed to bridge a gap introduced by newer versions of GlobalProtect 6.3.3.1-674+.

## Background

After upgrading GlobalProtect to version 6.3.3.1-674, VPN interfaces such as `gpd0` became unmanaged by NetworkManager. As a result, previously functioning mechanisms for detecting VPN state transitions ("VPN connected" and "VPN disconnected") stopped working. This made it difficult to trigger additional host-side automation whenever the VPN state changed. It was found that GlobalProtect updates shell scripts within its working directory `/opt/paloaltonetworks/globalprotect/network/config` during VPN lifecycle events. Monitoring these activities provides a reasonably reliable (though imperfect) way to detect when the VPN comes up or goes down.

## Solution

`systemd-if`, together with appropriately configured systemd.path units, enables event-driven execution of custom systemd services when:

- the VPN becomes connected ("VPN ON")
- the VPN becomes disconnected ("VPN OFF")

The utility acts as a conditional gateway between a triggering event and a target systemd unit. When condition evaluates to true, it forwards the requested action (`start`, `stop`, `restart`, `status`, etc.) to the specified target unit.

## Design Goals

`systemd-if` was designed with minimal host impact in mind:

- statically linked executable
- small binary footprint
- minimal runtime overhead
- no long-running daemons

## Typical Workflow

1. A systemd.path unit monitors files touched by GlobalProtect.
2. A filesystem event triggers a custom service.
3. The service invokes `systemd-if` with parameters.
4. `systemd-if` evaluates its condition.
5. If the condition is satisfied, the requested action is forwarded to the target systemd unit.

## Use Cases

- Triggering automation when a VPN connection is established.
- Triggering cleanup tasks when a VPN connection is terminated.
- Managing services that should only run while connected to corporate VPN.
- Working around VPN state detection limitations introduced by newer GlobalProtect releases.

## License

See the LICENSE file for details.
