#!/bin/bash

echo "It is a compilation of the second program..."
gcc -o main2 main2.c -lrt -lpthread

echo "Now we are cleaning the previous resources..."
./main2 controller 3 &
CONTROLLER_PID=$!
sleep 2
kill $CONTROLLER_PID 2>/dev/null

echo "Starting controller..."
./main2 controller 3 &
CONTROLLER_PID=$!

echo "Controller's PID: $CONTROLLER_PID"
sleep 3

echo "Starting chatty people..."
for i in {0..2}; do
    gnome-terminal --title="Person $i" -- ./main2 person $i 3
    sleep 1
done

echo "All processes have started! Hurray! :)"
echo "Our controller is running in background with PID: $CONTROLLER_PID"
echo "In order if you want to stop the program type: kill $CONTROLLER_PID"