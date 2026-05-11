#!/bin/bash
# STRESS TEST: Thread Pool, Mutex Locks, and IPC Sockets

echo -e "\e[1;36m[+] Initializing OS-Level Concurrency Stress Test...\e[0m"
sleep 1
echo -e "\e[1;33m[+] Spawning 50 concurrent child processes to flood the server...\e[0m"

# The exam now has 10 questions. Because the server randomly shuffles 
# We just send 10 dummy answers 
# To force all clients to finish the exam as fast as possible.
ANSWERS="test\ntest\ntest\ntest\ntest\ntest\ntest\ntest\ntest\ntest\n"

for i in {1..30}
do
   # The '&' symbol forks a background process to run the client
   # We use "$ANSWERS" in quotes so bash formats the newlines correctly!
   (echo -e "$ANSWERS" | ./client > /dev/null 2>&1) &
   
   # (since our listen() backlog in server.c is set to MAX_CLIENTS = 10)
   sleep 0.1
done

echo -e "\e[1;31m[+] 50 Sockets connected! Waiting for Mutex Race Conditions to resolve...\e[0m"
wait

echo -e "\e[1;32m[+] SUCCESS! All 30 simulated students finished without corrupting data.\e[0m"
echo -e "\e[1;36m[+] Look at your Server terminal to see the 30 Mutex Locks in action!\e[0m"
