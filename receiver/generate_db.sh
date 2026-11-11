#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
sqlite3 Navtex.db < "$script_dir/generate_db.sql"
