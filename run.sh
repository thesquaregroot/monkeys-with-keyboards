#!/bin/bash

# on SIGINT, send SIGTERM to monkey
# this is needed because system() calls block SIGINT
_term() {
  kill -TERM "$child" 2>/dev/null
}

trap _term SIGINT

./monkey $@ &> out.log &
child="$!"

wait "$child"

