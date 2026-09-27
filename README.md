# systemd-if

`systemd-if` is a lightweight conditional wrapper for `systemctl` designed to bridge a gap introduced by newer versions of GlobalProtect 6.3.3.1-674+.

## Background

After upgrading GlobalProtect to version 6.3.3.1-674, VPN interfaces such as `gpd0` became unmanaged by NetworkManager. As a result, previously functioning mechanisms for detecting VPN state transitions ("VPN connected" and "VPN disconnected") stopped working. This made it difficult to trigger additional host-side automation whenever the VPN state changed. It was found that GlobalProtect updates shell scripts within its working directory `/opt/paloaltonetworks/globalprotect/network/config` during VPN lifecycle events. Monitoring these activities provides a reasonably reliable (though imperfect) way to detect when the VPN comes up or goes down.

## Solution

`systemd-if`, together with appropriately configured systemd.path units, enables event-driven execution of custom systemd services when:

- the VPN becomes connected ("VPN ON")
- the VPN becomes disconnected ("VPN OFF")

The utility acts as a conditional gateway between a triggering event and a target systemd unit. When condition evaluates to true, it forwards the requested action (`start` or `stop`) to the specified target unit.

## Design Goals

`systemd-if` was designed with minimal host impact in mind:

- minimal run overhead
- statically linked executable
- small binary footprint
- no daemons

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

## Caveats

Although systemd.path units ([gpd0-up.path](systemd/gpd0-up.path) and [gpd0-down.path](systemd/gpd0-down.path)) may trigger custom services ([gpd0-up.service](systemd/gpd0-up.service), [gpd0-down.service](systemd/gpd0-down.service)) multiple times, the target [gpd0.service](systemd/gpd0.service) is protected by systemd logic and will not actually start or stop more than once.

## Verification of the authenticity

```sh
export VERSION=0.0.3
curl --fail --show-error --silent --location --remote-name \
  "https://github.com/omilevskyi/systemd-if/releases/download/v${VERSION}/CHECKSUM_${VERSION}.sha256.sigstore.json"
cosign verify-blob \
  --certificate-identity https://github.com/omilevskyi/systemd-if/.github/workflows/release.yml@refs/heads/main \
  --certificate-oidc-issuer https://token.actions.githubusercontent.com \
  --bundle "CHECKSUM_${VERSION}.sha256.sigstore.json" \
  "https://github.com/omilevskyi/systemd-if/releases/download/v${VERSION}/CHECKSUM_${VERSION}.sha256"
```

## License

BSD 3-Clause, see the LICENSE file for details.
