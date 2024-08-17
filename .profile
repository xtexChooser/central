# ZVM
export PATH="/opt/zig/bin:$PATH"

export PATH="$HOME/.local/bin:$PATH"

[ -e /usr/bin/vim ] && export EDITOR=vim
[ -e /usr/bin/nvim ] && export EDITOR=nvim

export ZVM_INSTALL="$HOME/.zvm/self"
export PATH="$PATH:$HOME/.zvm/bin"
export PATH="$PATH:$ZVM_INSTALL/"

