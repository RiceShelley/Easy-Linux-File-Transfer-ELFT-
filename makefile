CC=gcc
CFLAGS=
EXECUTABLE=elft
CFILES= main.c host.c recv.c
SRCS= $(CFILES) host.h recv.h

all: $(SRCS) 
	$(CC) $(CFLAGS) $(CFILES) -o $(EXECUTABLE)

$(SRCS):
	@echo "Missing files"
	@exit 1

update:
	@echo "Pulling latest update from repo"
	@rm $(SRCS)
	@rm $(EXECUTABLE)
	@git clone https://www.github.com/rootieDev/Easy-Linux-File-Transfer-ELFT-
	@sudo mv Easy-Linux-File-Transfer-ELFT-/* .
	@sudo rm -r Easy-Linux-File-Transfer-ELFT-/
	@make all

install $(EXECUTABLE):
	sudo cp elft /usr/local/bin/

uninstall /usr/bin/elft:
	sudo rm /usr/local/bin/elft

clean: $(EXECUTABLE)
	@rm $(EXECUTABLE)

