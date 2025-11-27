#!/bin/bash

echo "Start"
echo "Compilation"
gcc -o main3 main3.c -lrt -lpthread
gcc -o observer observer.c -lrt -lpthread

echo "Cleaning up previous resources"

pkill main3 2>/dev/null
pkill observer 2>/dev/null
rm -f /dev/shm/sem.chatty_* 2>/dev/null
rm -f /dev/shm/chatterboxes_shm 2>/dev/null
ipcrm -a 2>/dev/null || true

echo "Starting controller..."
./main3 controller 3 &
CONTROLLER_PID=$!
sleep 3

echo "Starting observer..."
./observer &
OBSERVER_PID=$!
sleep 2

echo "Chatting started..."
for i in 0 1 2; do
    echo "Starting person $i"
    ./main3 person $i 3 &
    sleep 1
done

echo ""
echo "Started"
echo "Controller PID: $CONTROLLER_PID"
echo "Observer PID: $OBSERVER_PID"
echo ""
echo "To stop enter: kill $CONTROLLER_PID"
echo "Observer is displaying all"
echo ""