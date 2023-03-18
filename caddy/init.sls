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
        - require:
            - test: container
    docker_network.present:
        - driver: bridge
        - ipam_driver: default
        - ipam_opts: driver=host-local
    docker_container.running:
        - image: ghcr.io/xtex-vnet/caddy:latest
        - binds:
            - /etc/caddy:/etc/caddy:ro
            - /var/run:/var/run
            - /var/lib/caddy:/root/.local/share/caddy
        - port_bindings:
            - 80:80
            - 443:443
        - cap_add: CAP_NET_BIND_SERVICE
        - networks:
            - caddy:
                - aliases: []
        - require:
            - test: container
            - docker_image: caddy
            - docker_network: caddy
            - file: caddy
        - hostname: caddy
        - environment:
            - HOME=/root
        - watch:
            - file: caddy
        - memory: 32M
