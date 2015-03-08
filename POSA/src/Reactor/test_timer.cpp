#include <ace/Log_Msg.h>
#include <ace/Reactor.h>
#include <ace/Null_Condition.h>
#include <ace/Message_Block.h>
#include <ace/Message_Queue.h>

class TimerHandler : public ACE_Event_Handler
{
public:
  TimerHandler (int timer = -1);

  int timer_;

  virtual int handle_timeout(ACE_Time_Value const &, void const * arg);
};

TimerHandler::TimerHandler (int timer) : timer_(timer)
{
}

int TimerHandler::handle_timeout(ACE_Time_Value const &, void const * arg)
{
  if (timer_ == -1)
  {
    ACE_DEBUG((LM_INFO, ACE_TEXT("%T handle_timeout\n")));
  } else {
    ACE_DEBUG((LM_INFO, ACE_TEXT("%T resetting timer...\n")));
    ACE_Time_Value timer_timeout;
    timer_timeout.msec(5000);
    ACE_Reactor::instance ()->reset_timer_interval(timer_, timer_timeout);
    return -1;
  }
  return 0;
}

int ACE_TMAIN (int, ACE_TCHAR *[])
{
  ACE_Time_Value timer_timeout;

  TimerHandler handler;
  timer_timeout.msec(2000);
  int timer = ACE_Reactor::instance ()->schedule_timer(&handler, NULL, timer_timeout, timer_timeout);

  TimerHandler handler2(timer);
  timer_timeout.msec(7000);
  ACE_Reactor::instance ()->schedule_timer(&handler2, NULL, timer_timeout, timer_timeout);

  ACE_Reactor::instance ()->run_reactor_event_loop ();

  return (0);
}
