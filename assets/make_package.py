#!/usr/bin/env python

import glob
from PIL import Image
from math import floor
import os.path
import io

palette = [[0,0,0]];
transparency = 0

def transform( pixel ):
    
    if ((pixel[3] < 255) ):
 #       print("skipping")
        return transparency;
       
    shade = 0;  
    shade += (((((pixel[0] & 255) ) << 2) >> 8)) << 6;
    shade += (((((pixel[1] & 255) ) << 3) >> 8)) << 3;
    shade += (((((pixel[2] & 255) ) << 3) >> 8)) << 0;

    print "R: " + str(pixel[0]) + " G: " + str(pixel[1]) + " B: " + str(pixel[2]) + " became " + str(shade)
    return shade
                    
def palettize_file( filename ):

    output_filename = filename.replace( os.path.splitext(filename)[1], ".img").\
        replace("src/", "img/")
    print( str(filename) + " became " + str(output_filename ))
    img_src = Image.open( filename ).convert('RGBA')
    img_data = img_src.load()
    img_dst = io.open( output_filename, "wb")
    img_dst.write(bytearray([(img_src.width & 0xFF00) >> 8, img_src.width &
                             0xFF, (img_src.height & 0xFF00) >> 8,
                             (img_src.height & 0xFF)]))
    
    for y in range( 0, img_src.height ):
        for x in range( 0, img_src.width ):
            pixel = img_data[ x, y ]
            adjusted = transform( pixel )
#            print( "transforming " + str(pixel) + " into " + str(adjusted) )
            img_dst.write(bytearray([adjusted]))
               
if __name__ == "__main__":              

    transparency = 199
    palettize_file( "src/tile2.png" )
#    for filename in glob.glob('src/*.png'):
#        palettize_file( filename )
                
