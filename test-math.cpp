#include <lv.h>

using namespace std;

int main(int argc, char** argv)
{
    // BLAS
    var bt;
    bt = 1.0f, 1.2f, 0.8f, -2.0f;
    cout << bt << "  sums to " << lv::sum(bt)  << endl;
    cout << bt << " asums to " << lv::asum(bt) << endl;

    // Broadcasting scalars
    var t1 = lv::irange(16.0f).view({4, 4});
    t1(1,2) = 2.3f;
    t1 += 1;
    cout << "t1: " << t1 << endl;
    t1 *= 1.5;
    cout << "t1: " << t1 << endl;
    t1 += t1;
    cout << "t1: " << t1 << endl;
    t1 -= t1 - 1;
    cout << "t1: " << t1 << endl;

    // Sub-views
    var r12 = lv::irange(0.1, 12.1);
    cout << "r12: " << r12 << endl;
    var sv = r12.view({4}, 4);
    cout << "sv: " << sv << endl;
    cout << "sv(0): " << sv(0) << endl;
    sv += 1;
    cout << "r12: " << r12 << endl;
    var rv;
    rv = -1.0, -2.0, -3.0, -4.0;
    cout << "rv: " << rv << endl;
    sv.offset(8);
    sv = rv;
    cout << "r12: " << r12 << endl;

    // Broadcasting tensors
    var r6 = lv::irange(6.0).view({3,2});
    var r2;
    r2 = 1.0, 2.0;
    cout << "r2: " << r2 << endl;
    cout << "r6: " << r6 << endl;
    r6 += r2;
    cout << "r6: " << r6 << endl;
    r6 *= r2;
    cout << "r6: " << r6 << endl;
    cout << "r6*r6: " << r6 * r6 << endl;

    // View of a view
    var r3 = r6.view({2}, 0);
    var r4 = sv.view({2}, 0);
    cout << "r3: " << r3 << endl;
    cout << "r4: " << r4 << endl;

    // Functors
    cout << "Size: " << sizeof(lv::Pow) << endl;
    cout << "Pow: " << lv::pow(3.0, 2) << endl;
    cout << "Pow: " << lv::pow(r6, 2) << endl;
    cout << "Tan: " << lv::tan(r6) << endl;

    // Transpose
    cout << "oTranspose:\n" << lv::transpose(r6) << endl << r6 << endl;
    cout << "iTranspose:\n" << r6.transpose() << endl << r6 << endl;

    // Dot
    var rd;
    rd = 2.0, 2.0, 2.0;
    var rd6 = lv::dot(r6, rd);
    cout << "Dot: " << rd6 << endl;

    // DFT
    var td = lv::view({2, 10});
    var fd;
    for (int i=0; i<10; i++)
    {
        td(0, i) = sinf(i);
        td(1, i) = cosf(i);
    }
    cout << "Time: " << td << endl;
    lv::DFT dft(10);
    fd = dft(td);
    cout << "Freq: " << fd << endl;
    var ab = lv::abs(fd);
    cout << "Abs: " << ab << endl;

    var im = lv::iamax(fd);
    cout << "IAMax: " << im << endl;

    // Done
    return 0;
}
