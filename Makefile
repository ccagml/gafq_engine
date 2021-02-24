# game: game_config.o game.o
# 	g++ -o game game.o game_config.o -I /root/gafq_engine/b/includedir -L/root/gafq_engine/b/libdir -lboost_program_options -lboost_system -lboost_filesystem 

game: game.cpp game_config.cpp game_config.h
	# g++ -Wall -g -std=c++11 game.cpp game_config.cpp -o game -I /root/gafq_engine -L/root/gafq_engine/3rd/mysqlcapi/lib -I/root/gafq_engine/3rd/mysqlcapi/include -lmysqlclient -lboost_program_options -lboost_system -lboost_filesystem
	g++ -Wall -g -std=c++11 game.cpp game_config.cpp -o game -I /root/gafq_engine -L/root/gafq_engine/3rd/mysqlcapi/lib -I/root/gafq_engine/3rd/mysqlcapi/include -lmysqlclient -lboost_program_options -lboost_system -lboost_filesystem  -lpthread -lboost_thread

chat_client: chat_client.cpp socket_msg.h
	g++ -Wall -g3 -O0 -ggdb -std=c++11 chat_client.cpp -o chat_client -I /root/gafq_engine -lboost_program_options -lboost_system -lboost_filesystem -lpthread -lboost_thread

game_config: game_config.cpp game_config.h
	g++ -Wall -std=c++11 game_config.cpp -o game_config -I /root/gafq_engine -I /root/gafq_engine/b/includedir -L/root/gafq_engine/b/libdir -lboost_program_options -lboost_system -lboost_filesystem 

script_engine: script_engine.cpp
	g++ -Wall -Werror -g -std=c++11 script_engine.cpp -o script_engine -I /root/gafq_engine -I /root/gafq_engine/3rd/gafq/include -L/root/gafq_engine/3rd/gafq/lib -lboost_program_options -lboost_system -lboost_filesystem -lpthread -lboost_thread


test_db_mysql_c:test_db_mysql_c.cpp
	g++ -Wall -std=c++11 test_db_mysql_c.cpp -o test_db_mysql_c  -L/root/gafq_engine/3rd/mysqlcapi/lib -I/root/gafq_engine/3rd/mysqlcapi/include -I/root/gafq_engine/3rd/mysqlcapi/include/mysql -lmysqlclient

test_game_eng:test_game_eng.cpp
	g++ -Wall -std=c++11  test_game_eng.cpp -o test_game_eng -L/root/gafq_engine/3rd/gafq/lib -I/root/gafq_engine/3rd/gafq/include -lgafq -ldl

clean:
	 rm game game_config.o game_config test_db_mysql_c test_game_eng script_engine