if status is-interactive
    # Commands to run in interactive sessions can go here
end

# ZVM
export ZVM_INSTALL="$HOME/.zvm/self"
export PATH="$PATH:$HOME/.zvm/bin"
export PATH="$PATH:$ZVM_INSTALL/"

alias g=git

if command -q zoxide
	zoxide init fish | source
end

if command -q opam && test -e ~/.opam/opam-init/init.fish
    source ~/.opam/opam-init/init.fish > /dev/null 2> /dev/null; or true
end

if command -q rustup && test -e /opt/rust/bin/env.fish
    . /opt/rust/bin/env.fish
end
