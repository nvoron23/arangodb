#!/bin/bash
rm -rf cluster
mkdir cluster
cd cluster
echo Starting agency...
../bin/etcd-arango 2>&1 > /dev/null &
cd ..
sleep 1
echo Initializing agency...
bin/arangosh --javascript.execute scripts/init_agency.js
echo Starting discovery...
bin/arangosh --javascript.execute scripts/discover.js 2>&1 > cluster/discover.log &

start() {
    TYPE=$1
    PORT=$2
    mkdir cluster/data$PORT
    echo Starting $TYPE on port $PORT
    echo bin/arangod --database-directory cluster/data$PORT \
                --cluster.agency-endpoint tcp://localhost:4001 \
                --cluster.my-address tcp://localhost:$PORT \
                --server.endpoint tcp://localhost:$PORT \
                --cluster.my-local-info $TYPE:localhost:$PORT \
                --log.file cluster/$PORT.log 2>&1 > cluster/$PORT.stdout &
}

start dbserver 8629
start coordinator 8530

echo Waiting for cluster to come up...

sleep 10

echo Bootstrapping DBServers...
curl -X POST "http://localhost:8530/_admin/cluster/bootstrapDbServers" -d '{"isRelaunch":false}'

echo Running DB upgrade on cluster...
curl -X POST "http://localhost:8530/_admin/cluster/upgradeClusterDatabase" -d '{"isRelaunch":false}'

echo Bootstrapping Coordinators...
curl -X POST "http://localhost:8530/_admin/cluster/bootstrapCoordinator" -d '{"isRelaunch":false}'

echo Done, your cluster is ready at
echo "   bin/arangosh --server.endpoint tcp://localhost:8530
