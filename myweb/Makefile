.PHONY: all install clean

all: build install

build:
	make myweb

clean:
	rm myweb -f
	sudo rm /usr/local/bin/myweb -f
	sudo rm /usr/local/bin/myweb-wrap -f
	sudo rm -r /usr/local/bin/webpages
	sudo rm /etc/xinetd.d/myweb-xinetd -f
	sudo systemctl restart xinetd

install:
	sudo cp myweb /usr/local/bin
	sudo cp myweb-wrap /usr/local/bin
	sudo cp -r webpages /usr/local/bin
	sudo cp myweb-xinetd /etc/xinetd.d/
	sudo systemctl restart xinetd
