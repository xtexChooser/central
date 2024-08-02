if status is-interactive
    # Commands to run in interactive sessions can go here
end

alias g=git
alias gt=goto

function 0file; curl -F"file=@$argv" https://envs.sh; end
function 0pb; curl -F"file=@-;" https://envs.sh; end
function 0url; curl -F"url=$argv" https://envs.sh; end
function 0short; curl -F"shorten=$argv" https://envs.sh; end

# bun
set --export BUN_INSTALL "$HOME/.bun"
set --export PATH $BUN_INSTALL/bin $PATH

zoxide init fish | source

# pnpm
set -gx PNPM_HOME "/home/xtex/.local/share/pnpm"
if not string match -q -- $PNPM_HOME $PATH
  set -gx PATH "$PNPM_HOME" $PATH
end
# pnpm end

