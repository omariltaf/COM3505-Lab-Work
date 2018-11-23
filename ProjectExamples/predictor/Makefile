# Makefile -- Mark's predictive text stuff adapted to generate C++ model

default: predict foodflows.h c-loop

c-loop:
	bash -c "while :; do clear; make predict |grep -v 'ing directory'; ./predict; rm predict; read; done"
py-loop:
	bash -c "while :; do clear; python3 build-word-tables.py |fold -s -w 200; echo; read; done"
predict: predictor.h predictor.cpp Makefile
	@echo
	g++ -O3 -I. -std=c++11 -DPREDICTOR_MAIN=1 -o predict predictor.cpp
predictor.h: build-word-tables.py words-by-freq.txt
	python3 build-word-tables.py >$@
foodflows.h: food-words.txt build-word-tables.py predictor.h Makefile
	set `wc -l food-words.txt`; \
	head -"$$(expr 60 - $$1)" words-by-freq.txt >/tmp/$$$$; \
	cat $< >>/tmp/$$$$; \
	python3 build-word-tables.py /tmp/$$$$ | \
        sed -e 's,PREDICTOR,FOODFLOWS,' -e 's,predictor,foodflows,' >$@
