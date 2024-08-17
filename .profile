export PATH="$HOME/.local/bin:$PATH"

[ -e /usr/bin/vim ] && export EDITOR=vim
[ -e /usr/bin/nvim ] && export EDITOR=nvim

# ZVM
if [[ -e $HOME/.zvm ]]; then
    export ZVM_INSTALL="$HOME/.zvm/self"
    export PATH="$PATH:$HOME/.zvm/bin:$ZVM_INSTALL"
else
    export PATH="/opt/zig/bin:$PATH"
fi
