g++ -framework sfml -I include src/*.cpp -lsfml-graphics -lsfml-window -lsfml-system -o Acai
sips -i images/grape_emoji.png
/Applications/Xcode.app/Contents/Developer/Tools/DeRez -only icns images/grape_emoji.png > tmpicon.rsrc
/Applications/Xcode.app/Contents/Developer/Tools/Rez -append tmpicon.rsrc -o Acai
/Applications/Xcode.app/Contents/Developer/Tools/SetFile -a C Acai
rm tmpicon.rsrc
