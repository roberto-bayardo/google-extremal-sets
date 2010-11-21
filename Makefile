CFLAGS= -DNDEBUG -O3 -D_FILE_OFFSET_BITS=64
#CFLAGS= -g -D_FILE_OFFSET_BITS=64 -Wall -Wextra
CC = g++

.SUFFIXES: .o .cc

LIBS =

OBJS_c = data-source-iterator.cc set-properties.cc

OBJS_lexicographic_c = all-maximal-sets-lexicographic.cc main-lexicographic.cc $(OBJS_c)
OBJS_lexicographic_o = $(OBJS_lexicographic_c:.cc=.o)

OBJS_cardinality_c = all-maximal-sets-cardinality.cc main-cardinality.cc $(OBJS_c)
OBJS_cardinality_o = $(OBJS_cardinality_c:.cc=.o)

OBJS_satelite_c = all-maximal-sets-satelite.cc main-satelite.cc $(OBJS_c)
OBJS_satelite_o = $(OBJS_satelite_c:.cc=.o)

OBJS_sorter_c = main-sorter.cc sorter.cc $(OBJS_c)
OBJS_sorter_o = $(OBJS_sorter_c:.cc=.o)

OBJS_dimacs-to-apriori_c = main-dimacs-to-apriori.cc dimacs-to-apriori.cc $(OBJS_c)
OBJS_dimacs-to-apriori_o = $(OBJS_dimacs-to-apriori_c:.cc=.o)

OBJS_item-fixer_c = main-item-fixer.cc item-fixer.cc $(OBJS_c)
OBJS_item-fixer_o = $(OBJS_item-fixer_c:.cc=.o)

all: ams-lexicographic ams-cardinality ams-satelite

ams-lexicographic: $(OBJS_lexicographic_c) $(OBJS_lexicographic_o)
		$(CC) $(CFLAGS) $(LINKFLAGS) -o ams-lexicographic $(OBJS_lexicographic_o) $(LIBS)

ams-cardinality: $(OBJS_cardinality_c) $(OBJS_cardinality_o)
		$(CC) $(CFLAGS) $(LINKFLAGS) -o ams-cardinality $(OBJS_cardinality_o) $(LIBS)

ams-satelite: $(OBJS_satelite_c) $(OBJS_satelite_o)
		$(CC) $(CFLAGS) $(LINKFLAGS) -o ams-satelite $(OBJS_satelite_o) $(LIBS)

sorter: $(OBJS_sorter_c) $(OBJS_sorter_o)
		$(CC) $(CFLAGS) $(LINKFLAGS) -o sorter $(OBJS_sorter_o) $(LIBS)

dimacs-to-apriori: $(OBJS_dimacs-to-apriori_c) $(OBJS_dimacs-to-apriori_o)
		$(CC) $(CFLAGS) $(LINKFLAGS) -o dimacs-to-apriori $(OBJS_dimacs-to-apriori_o) $(LIBS)

item-fixer: $(OBJS_item-fixer_c) $(OBJS_item-fixer_o)
		$(CC) $(CFLAGS) $(LINKFLAGS) -o item-fixer $(OBJS_item-fixer_o) $(LIBS)

.cc.o:
	$(CC) $(CFLAGS) -c $<

depend:
	$(CC) -MM -c $(OBJS_lexicographic_c) $(OBJS_cardinality_c) $(OBJS_sorter_c) $(OBJS_dimacs-to-apriori) $(OBJS_item-fixer_c) > Makefile.dependencies

clean:
	rm ams-* item-fixer dimacs-to-apriori sorter *.o

-include Makefile.dependencies
