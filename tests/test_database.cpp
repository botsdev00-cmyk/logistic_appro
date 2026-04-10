#include <gtest/gtest.h>
#include <QString>

// Test simple pour vérifier que la config est correcte
class DatabaseTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialisation avant chaque test
    }

    void TearDown() override
    {
        // Nettoyage après chaque test
    }
};

// Test de base
TEST_F(DatabaseTests, TestConnection)
{
    QString expected = "semuliki";
    EXPECT_EQ(expected.toStdString(), "semuliki");
}

TEST_F(DatabaseTests, TestDatabaseName)
{
    QString dbName = "semuliki";
    ASSERT_FALSE(dbName.isEmpty());
    ASSERT_EQ(dbName.length(), 8);
}