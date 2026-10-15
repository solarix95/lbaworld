#include <QtTest/QtTest>

class TestLiblbadata: public QObject
{
    Q_OBJECT

private slots:
    void testBinWriter();
    void testBinReader();
    void testBinReaderErrors();

    void testHqrFile();
    void testHqrLazyFileAccess();
    void testHqrValidation();
    void testLbaRess();
    void testLbaAnimation();
    void testLbaCharacter();

    void testLbaPalette();
};
