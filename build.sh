clang -DPACKAGE_NAME=\"paqrss\" -DPACKAGE_TARNAME=\"paqrss\" -DPACKAGE_VERSION=\"0.01\" -DPACKAGE_STRING=\"paqrss\ 0.01\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DPACKAGE=\"paqrss\" -DVERSION=\"0.01\" -I. -Wall -g -O2 -MT paqrss.o -MD -MP -MF .deps/paqrss.Tpo -c -o paqrss.o paqrss.c
mv -f .deps/paqrss.Tpo .deps/paqrss.Po
clang -Wall -g -O2   -o paqrss paqrss.o  -lfftw3 -lm  -lfreeimage -lpulse -lpulse-simple

