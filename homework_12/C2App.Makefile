C2App/start-sim:
	cd homework_12 && \
	docker compose -f sim/compose.sitl.yml up -d --build

C2App/start-edge:
	cd homework_12 && \
	docker compose -f edge/docker-compose.yml up --build --force-recreate

C2App/logs:
	docker exec -it edge-c2_service-1 tail -n 50 -f /var/log/c2/c2.log