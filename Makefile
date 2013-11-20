DOVDIR:=../dovecot-2.0.13
LIBS:=-ldrac
LDFLAGS:=-L/usr/local/lib

CFLAGS=-Wall -W -shared -fPIC -DHAVE_CONFIG_H -I$(DOVDIR) \
	-I$(DOVDIR)/src/lib \
	-I$(DOVDIR)/src/lib-index \
	-I$(DOVDIR)/src/lib-mail \
	-I$(DOVDIR)/src/lib-storage \
	-I$(DOVDIR)/src/lib-storage/index \
	-I$(DOVDIR)/src/lib-storage/index/maildir

all: drac_plugin.so

drac_plugin.so: drac-plugin.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(LIBS)

clean:
	@rm -f *.so *.o
