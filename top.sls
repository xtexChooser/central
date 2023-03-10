base:
    '*': 
        - container
        - router.bird
        - router.ip_fwd
        - router.wireguard
        - firewalld.base
        - caddy
    'service:salt-master:true':
        - match: pillar
        - salt-master-cd
    'service:ca:true':
        - match: pillar
#    'service:etcd:true':
#        - match: pillar
#        - etcd.base
    'service:ntfy:true':
        - match: pillar
        - ntfy
