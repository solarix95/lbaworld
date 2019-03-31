#include <QtTest/QtTest>

class TestLiblbadata: public QObject
{
    Q_OBJECT

private slots:
    void testBinWriter();
    void testBinReader();

    void testHqrFile();

    void testLbaPalette();
};
