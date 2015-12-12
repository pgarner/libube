#include <fstream>
#include <lube.h>

using namespace std;

int main(int argc, char** argv)
{
    // Read a small file
    var f;
    var t;
    ifstream is(TEST_DIR "/test.txt", ifstream::in);
    if (is.fail())
        cout << "Open failed" << endl;
    while (f.getline(is))
    {
        cout << f << endl;
        t.push(f.copy());
    }
    cout << t << endl;

    // Read the same text file via a dynamic library
    file txtf("txt");
    var txt = txtf.read(TEST_DIR "/test.txt");
    cout << "Loaded: " << txt << endl;

    // Read a .ini file via a dynamic library
    file inif("ini");
    var ini = inif.read(TEST_DIR "/test.ini");
    cout << "Loaded: " << ini << endl;

    // Init from comma separated list
    var ai;
    ai = 5,4,3,2,1;
    cout << "ai is: " << ai << endl;

    // Plot it with gnuplot
    var gnu;
    gnu.push("plot sin(x), \"-\"");
    gnu.push(ai);
    file gnuf("gnuplot");
    gnuf.write("test.eps", gnu);

    // gedcom
    file gedf("ged");
    var ged = gedf.read(TEST_DIR "/test.ged");
    cout << "Loaded: " << ged << endl;

    // XML
    file xmlf("xml");
    var xml = xmlf.read(TEST_DIR "/test.xml");
    cout << "Loaded: " << xml << endl;
    xmlf.write("test-out.xml", xml);

    // wav
    var sndAttr;
    sndAttr[lube::nil];
    file wavf("snd", sndAttr);
    var wav = wavf.read(TEST_DIR "/test.wav");
    int dim = wav.dim();
    cout << "Loaded wav file:" << endl;
    cout << " rate:     " << sndAttr["rate"] << endl;
    cout << " channels: " << (dim > 1 ? wav.shape(0) : 1) << endl;
    cout << " frames:   " << wav.shape(dim-1) << endl;

    // Done
    return 0;
}
