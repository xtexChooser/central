function 0file; curl -F"file=@$argv" https://envs.sh; end
function 0pb; curl -F"file=@-;" https://envs.sh; end
function 0url; curl -F"url=$argv" https://envs.sh; end
function 0short; curl -F"shorten=$argv" https://envs.sh; end
