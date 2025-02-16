release level="patch":
    @maint/publish.sh {{level}}

alias bc := build-client

default: build-client

build-client:
    @cargo build -p yjyz-tools --target x86_64-pc-windows-gnu

build-client-release:
    @cargo build -p yjyz-tools --target x86_64-pc-windows-gnu --release

alias sd := sync-dist
sync-dist:
    @rsync -rcvzP --delete maint/dist/ envs.net:public_html/yjyz-tools/
