
make clean
make game
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/gafq_engine/3rd/mysqlcapi/lib:/root/gafq_engine/3rd/gafq/lib
./game --props ./config.json