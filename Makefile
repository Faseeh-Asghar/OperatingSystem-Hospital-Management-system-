all: logs admissions patient_simulator dashboard

# make the logs folder if it doesn't exist
logs:
	mkdir -p logs

# Build the admissions manager
admissions: src/admissions.c
	gcc -Wall -Wextra -pthread -o admissions src/admissions.c

# Build the patient simulator
patient_simulator: src/patient_simulator.c
	gcc -Wall -Wextra -pthread -o patient_simulator src/patient_simulator.c

dashboard: src/dashboard.c
	gcc -Wall -Wextra -o dashboard src/dashboard.c

# remove all compiled files
clean:
	rm -f admissions patient_simulator dashboard
	rm -f logs/schedule_log.txt
