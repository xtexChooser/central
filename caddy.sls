caddy:
  file.managed:
    - name: /etc/caddy/Caddyfile
    - source: salt://caddy/Caddyfile.j2
    - context:
        tpldir: caddy/
    - template: jinja
    - user: root
    - group: root
    - mode: "0666"
    - makedirs: True
  docker_image.present:
    - name: ghcr.io/xtex-vnet/caddy
    - tag: latest
    - force: True
    - require:
      - test: container
  docker_container.running:
    - image: ghcr.io/xtex-vnet/caddy:latest
    - binds:
      - /etc/caddy:/etc/caddy:ro
      - /var/run:/var/run
      - /var/lib/caddy:/root/.local/share/caddy
    - publish_all_ports: True
    - network_mode: host
    - cap_add: CAP_NET_BIND_SERVICE
    - require:
      - test: container
      - docker_image: caddy
      - file: caddy
    - environment:
      - HOME=/root
      - HOSTNAME={{ grains['fqdn'] }}
    - watch:
      - file: caddy
