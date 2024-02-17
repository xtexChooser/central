#!/bin/env php
<?php
$versions = [
    '2.14',
    '3.0-alpha2',
    '2.13.1',
    '2.13',
]
?># GENERATED FILE, DO NOT MODIFY
# regenerate with ./pipeline.sh

steps:
    - name: container-build
      image: woodpeckerci/plugin-docker-buildx
      group: dry-build
      settings:
          dockerfile: Containerfile
          dry_run: true
          repo: codeberg.org/xvnet/bird
          tags: latest
          build_args:
              VERSION: <?= $versions[0] ?>


<?php
for($i = 0, $size = count($versions); $i < $size; ++$i) {
?>
    - name: build-<?= $versions[$i] ?>

      image: woodpeckerci/plugin-docker-buildx
      group: build
      settings:
          dockerfile: Containerfile
          platforms: linux/arm64/v8,linux/amd64
          repo: codeberg.org/xvnet/bird
          registry: codeberg.org
          tags:
              - <?= $versions[$i] ?>

<?php if ($i == 0) echo "              - latest\n" ?>
          build_args:
              VERSION: <?= $versions[$i] ?>

          username: ${CI_REPO_OWNER}
          password:
              from_secret: oci_token
      when:
          event: push

<?php
}
?>