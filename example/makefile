.PHONY:all
.PHONY:clean
all:bind timerfd timerWheel regex request

bind:bind.cpp
	g++ -o $@ $^ -std=c++17

timerfd:timerfd.cpp
	g++ -o $@ $^ -std=c++17

timerWheel:timerWheel.cpp
	g++ -o $@ $^ -std=c++17

regex:regex.cpp
	g++ -o $@ $^ -std=c++17

request:request.cpp
	g++ -o $@ $^ -std=c++17

clean:
	rm -f bind timerfd timerWheel regex request
