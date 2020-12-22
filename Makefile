BASE = asst5

OPT = 1
OS := $(shell uname -s)

all: $(BASE)

ifeq ($(OS), Linux)
  CPPFLAGS = 
  LDFLAGS +=
  LIBS += -lGL -lGLU -lglfw -lGLEW
endif

ifeq ($(OS), Darwin)  # macOS
  CXXFLAGS = -std=c++11
  # CXXFLAGS += -stdlib=libc++     # default on macOS
  CPPFLAGS += -I.
  LDFLAGS += -framework OpenGL -lglfw  -lGLEW 
endif

ifdef OPT 
  #turn on optimization
  CXXFLAGS += -O2
else 
  #turn on debugging
  CXXFLAGS += -g
endif

CXX = g++ 

OBJ = $(BASE).o ppm.o glsupport.o geometry.o material.o renderstates.o texture.o

$(BASE): $(OBJ)
	$(LINK.cpp) -o $@ $^ $(LIBS) 

clean:
	rm -f $(OBJ) $(BASE)

