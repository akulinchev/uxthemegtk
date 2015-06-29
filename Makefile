# This file is part of UxThemeGTK.
#
# Copyright © 2015 Ivan Akulinchev
#
# UxThemeGTK is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# UxThemeGTK is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with UxThemeGTK.  If not, see <http://www.gnu.org/licenses/>.

CC = winegcc
RM = rm -f

CFLAGS  = -fPIC -Wall -O3 $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = -shared -lgdi32 $(shell pkg-config --libs gtk+-3.0)

TARGET  = uxtheme.dll.so
SOURCES = $(shell echo src/*.c)
OBJECTS = $(SOURCES:.c=.o)
LIBSPEC = src/uxtheme.spec

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBSPEC)

clean:
	$(RM) $(TARGET) $(OBJECTS)

# vim: noexpandtab filetype=make