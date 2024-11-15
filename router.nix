{ ... }:
{
  boot.kernel = {
    sysctl = {
      "net.ipv4.conf.all.forwarding" = true;
      "net.ipv6.conf.all.forwarding" = false;
      "net.ipv4.conf.default.rp_filter" = 1;
      "net.ipv4.conf.wan.rp_filter" = 1;
      "net.ipv4.conf.br-lan.rp_filter" = 0;
    };
  };

  systemd.network = {
    wait-online.anyInterface = true;
    netdevs = {
      # Create the bridge interface
      "20-br-lan" = {
        netdevConfig = {
          Kind = "bridge";
          Name = "br-lan";
        };
      };
    };
    networks = {
      "30-lan0" = {
        matchConfig.Name = "lan0";
        linkConfig.RequiredForOnline = "enslaved";
        networkConfig = {
          ConfigureWithoutCarrier = true;
        };
      };
      # lan1 and lan2 look analogical
      "30-lan3" = {
        matchConfig.Name = "lan3";
        linkConfig.RequiredForOnline = "enslaved";
        networkConfig = {
          ConfigureWithoutCarrier = true;
        };
      };
      "10-wan" = {
        matchConfig.Name = "wan";
        networkConfig = {
          # start a DHCP Client for IPv4 Addressing/Routing
          DHCP = "ipv4";
          DNSOverTLS = true;
          DNSSEC = true;
          IPv6PrivacyExtensions = false;
          IPForward = true;
        };
        # make routing on this interface a dependency for network-online.target
        linkConfig.RequiredForOnline = "routable";
      };
    };
  };
  networking = {
    useNetworkd = true;
    useDHCP = false;

    # No local firewall.
    nat.enable = false;
    firewall.enable = false;

    nftables = {
      enable = true;
      ruleset = ''
        table inet filter {
          chain input {
            type filter hook input priority 0; policy drop;

            iifname { "br-lan" } accept comment "Allow local network to access the router"
            iifname "wan" ct state { established, related } accept comment "Allow established traffic"
            iifname "wan" icmp type { echo-request, destination-unreachable, time-exceeded } counter accept comment "Allow select ICMP"
            iifname "wan" counter drop comment "Drop all other unsolicited traffic from wan"
            iifname "lo" accept comment "Accept everything from loopback interface"
          }
          chain forward {
            type filter hook forward priority filter; policy drop;

            iifname { "br-lan" } oifname { "wan" } accept comment "Allow trusted LAN to WAN"
            iifname { "wan" } oifname { "br-lan" } ct state { established, related } accept comment "Allow established back to LANs"
          }
        }

        table ip nat {
          chain postrouting {
            type nat hook postrouting priority 100; policy accept;
            oifname "wan" masquerade
          }
        }
      '';
    };
  };

  services.hostapd = {
    enable = true;
    radios = {
      wlan0 = {
        band = "2g";
        countryCode = "US";
        channel = 0; # ACS

        wifi4.enable = true;

        networks = {
          wlan0 = {
            ssid = "wifi-name";
            authentication = {
              mode = "wpa3-sae";
              saePasswordsFile = ./ap-password;
            };
            bssid = "36:b9:ff:ff:ff:ff";
            settings = {
              bridge = "br-lan";
            };
          };
        };
      };
  };
};
}
