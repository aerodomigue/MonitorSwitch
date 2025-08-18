class Data {
public:
    Data();
    ~Data();

    void setPeripheralID(int id);
    int getPeripheralID() const;

private:
    int peripheralID;
};