#include <string>
#include <functional>
class Button {
public:
    Button(const std::string& label);
    void setLabel(const std::string& label);
    std::string getLabel() const;
    void onClick(std::function<void()> callback);

private:
    std::string label;
    std::function<void()> clickCallback;
};