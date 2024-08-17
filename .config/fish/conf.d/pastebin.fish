function 0file; curl -F"file=@$1" https://envs.sh; end
function 0pb; curl -F"file=@-;" https://envs.sh; end
function 0url; curl -F"url=$1" https://envs.sh; end
function 0short; curl -F"shorten=$1" https://envs.sh; end
