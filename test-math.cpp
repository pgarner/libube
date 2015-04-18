#include <lv.h>

using namespace std;

// A test unary functor
class Unary : public lv::UnaryFunctor
{
public:
    Unary() { mDim = 1; };
protected:
    void vector(var iVar, var& oVar) const;
};

void Unary::vector(var iVar, var& oVar) const
{
    for (int i=0; i<iVar.size(); i++)
        oVar[i] = iVar[iVar.size()-1-i];
    cout << "Unary: I: " << iVar.shape() << " " << iVar << endl;
    cout << "Unary: O: " << oVar.shape() << " " << oVar << endl;
}


// A test N-ary functor
class Nary : public lv::NaryFunctor
{
public:
    Nary() { mDim = 1; };
protected:
    void vector(var iVar, var& oVar) const;
};

void Nary::vector(var iVar, var& oVar) const
{
    for (int i=0; i<iVar[0].size(); i++)
        oVar[i] = iVar[0][iVar.size()-1-i];
    cout << "Nary[0]: " << iVar[0].shape() << " " << iVar[0] << endl;
    cout << "Nary[1]: " << iVar[1].shape() << " " << iVar[1] << endl;
    cout << "Nary[2]: " << iVar[2].shape() << " " << iVar[2] << endl;
    cout << "Nary: O: " << oVar.shape() << " " << oVar << endl;
}

int main(int argc, char** argv)
{
    // BLAS
    var bt;
    bt = 1.0f, 1.2f, 0.8f, -2.0f;
    cout << "dim: " << bt.dim() << " shape: " << bt.shape() << endl;
    cout << "shape(0): " << bt.shape(0)
         << " shape(-1): " << bt.shape(-1) << endl;
    cout << bt << "  sums to " << lv::sum(bt)  << endl;
    cout << bt << " asums to " << lv::asum(bt) << endl;

    // Broadcasting scalars
    var t1 = lv::irange(16.0f).view({4, 4});
    t1(1,2) = 2.3f;
    t1 += 1;
    cout << "t1: " << t1 << endl;
    cout << "t1*0.5: " << t1 * 0.5 << endl;
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
    cout << "sv[0]: " << sv[0] << endl;
    cout << "sv.size(): " << sv.size() << endl;
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
    lv::DFT idft(10, true);
    var ifd = idft(fd);
    cout << "IFreq: " << ifd << endl;

    // Check the complex operators
    cout << "Real: " << lv::real(fd) << endl;
    cout << "Imag: " << lv::imag(fd) << endl;
    cout << "Arg: " << lv::arg(fd) << endl;
    cout << "Abs: " << lv::abs(fd) << endl;
    cout << "Norm: " << lv::norm(fd) << endl;

    // Real output writing to complex storage
    var cd = var(12, lv::cfloat(0.0f,0.0f)).view({2,6});
    cout << "NormC: "<< lv::norm(fd, cd) << endl;

    var im = lv::iamax(fd);
    cout << "IAMax: " << im << endl;

    // Functor view broadcast
    Unary u;
    cout << u(r6) << endl;

    // N-ary functor
    var v1, v2, v3;
    v1 =
        0.0, 1.2, 1.4,
        1.0, 1.2, 1.4,
        2.0, 1.2, 1.4,
        3.0, 1.2, 1.4;
    v1 = v1.view({4,3});
    v2 =
        0.0, 2.2,
        1.0, 2.2,
        2.0, 2.2,
        3.0, 2.2;
    v2 = v2.view({4,2});
    v3 = 1.0, 2.2, 3.3, 4.4;
    Nary nary;
    var vN;
    vN.push(v1);
    vN.push(v2);
    vN.push(v3);
    cout << "vN: " << vN << endl;
    nary(vN);

    // Concatenate
    var vc = lv::concatenate({v1, v2});
    cout << "vc: " << v1 << v2 << vc << endl;

    // Polynomial stuff
    var xpoly({1.0f, 0.0f, 0.0f, -1.0f});
    var xroot = lv::roots(xpoly);
    cout << "Roots of x^3-1=0: " << xroot << endl;
    xpoly = lv::poly(xroot);
    cout << "Poly is: " << xpoly << endl;

    // Sort
    var fsrt({1.0, -1.0, 2.1, -2.1, 3.0});
    cout << "Usrt is: " << fsrt << endl;
    cout << "Sort is: " << lv::sort(fsrt, fsrt) << endl;
    var csrt("Super duper strING");
    cout << "Usrt is: " << csrt << endl;
    cout << "Sort is: " << lv::sort(csrt, csrt) << endl;

    // Done
    return 0;
}
