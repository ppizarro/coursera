import java.util.concurrent.Semaphore;
import java.util.concurrent.CountDownLatch;

public class PingPong
{
    public static int maxIterations = 40;
    
    // Latch will be decremented each time a thread exits.
    public static CountDownLatch latch = new CountDownLatch(2);

    // This class implements the ping/pong processing algorithm.
    public static class PlayPingPongThread extends Thread
    {
        public PlayPingPongThread (int maxIterations,
                                   String stringToPrint,
                                   Semaphore mySemaphore,
                                   Semaphore otherSemaphore)
        {
            this.maxIterations = maxIterations;
            this.stringToPrint = stringToPrint;
            this.mySemaphore = mySemaphore;
            this.otherSemaphore = otherSemaphore;
        }

        // Main event loop that runs in a separate thread of control.
        public void run () 
        {
            try {
                for (int loopsDone = 1;; ++loopsDone)
                    {
                        mySemaphore.acquire ();

                        System.out.println(stringToPrint + "(" + loopsDone + ")");

                        otherSemaphore.release ();

                        if (loopsDone == maxIterations)
                            {
                                latch.countDown();
                                return; // Bail out when we're done.
                            }
                    } 
            }
            catch (Exception e){}
        }

        // Data members that control the ping and pong operations.
        private int maxIterations;
        private String stringToPrint;
        protected Semaphore mySemaphore;
        protected Semaphore otherSemaphore;
    }

    public static void main(String[] args) {
        try {         
            // Create the ping and pong semaphores that control
            // alternation between threads.
            Semaphore pingSemaphore = new Semaphore(1);
            Semaphore pongSemaphore = new Semaphore(0);

            System.out.println("Ready...Set...Go!");

            // Create the ping and pong threads.
            PlayPingPongThread ping = new PlayPingPongThread(maxIterations, "Ping!", pingSemaphore, pongSemaphore);
            PlayPingPongThread pong = new PlayPingPongThread(maxIterations, "Pong!", pongSemaphore, pingSemaphore);
            
            // Initiate the ping and pong threads, which will call the
            // run() hook method.
            ping.start();
            pong.start();

            // Barrier synchronization to wait for both threads to
            // finish.
            latch.await();
        } 
        catch (java.lang.InterruptedException e)
            {}
        System.out.println("Done!");
    }
}
