#!/bin/bash

id=$(docker create godot-csound-download)
docker cp $id:/usr/share/sounds/sf2/FluidR3_GM.sf2 assets/FluidR3_GM.sf2
docker rm -v $id
