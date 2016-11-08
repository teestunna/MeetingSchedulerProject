class Meeting;

class Notification {
public:
  Notification(Meeting *meeting, bool scheduled);
  void display();
private:
  Meeting *meeting;
  bool scheduled;
};
