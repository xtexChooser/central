if status is-interactive
    # Commands to run in interactive sessions can go here
end

alias g=git

function 0file; curl -F"file=@$argv" https://envs.sh; end
function 0pb; curl -F"file=@-;" https://envs.sh; end
function 0url; curl -F"url=$argv" https://envs.sh; end
function 0short; curl -F"shorten=$argv" https://envs.sh; end

zoxide init fish | source

source ~/.opam/opam-init/init.fish > /dev/null 2> /dev/null; or true

