rm mycust_font.png
../png2bdf -f cust -o mycust.bdf -e 65 *.png
../../bdfconv/bdfconv -f 1 -b 0 -m '65-68' -v mycust.bdf -n mycust_font -o mycust_font.c -d ../../bdf/mycust.bdf -g 4
convert bdf.tga mycust_font.png
rm bdf.tga

