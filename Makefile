CXXFLAGS = -g -c -Istb -O6 -Ofast  -funroll-loops -funroll-all-loops -fomit-frame-pointer -fno-exceptions -fno-rtti -ffast-math -mtune=i486 -march=i486 -fpermissive

LDFLAGS = -lemu

CXX = i586-pc-msdosdjgpp-g++

OBJS = Main.o Game.o LoadImageDOS.o NativeBitmap.o MusicDOS.o DOSRenderer.o

TARGET = viana.exe

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm *~
