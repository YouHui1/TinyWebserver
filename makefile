CXX ?= g++

main: http/http_conn.cpp utils/utils.cpp main.cpp utils/config.cpp timer/timer.cpp webserver.cpp utils/log/log.cpp sql/sqlpool.cpp
	$(CXX)  $^ -o main -pthread -lmysqlclient
test: http/http_conn.cpp utils/utils.cpp main.cpp utils/config.cpp timer/timer_.cpp webserver.cpp utils/log/log.cpp sql/sqlpool.cpp
	$(CXX)  $^ -o main -pthread -lmysqlclient
main2: http/http_conn.cpp utils/utils.cpp main2.cpp utils/config.cpp timer/timer.cpp webserver.cpp utils/log/log.cpp sql/sqlpool.cpp
	$(CXX)  $^ -o main -pthread -lmysqlclient
