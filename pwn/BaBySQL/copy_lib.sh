#!/bin/bash
sudo $(which cp) boogy.so /usr/lib/mysql/plugin/
sudo $(which chmod) 0644 /usr/lib/mysql/plugin/boogy.so
ls -l $_
