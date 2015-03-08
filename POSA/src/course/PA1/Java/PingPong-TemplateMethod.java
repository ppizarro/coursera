import java.util.concurrent.Semaphore;
import java.util.concurrent.CountDownLatch;

public class PingPong
{
    public static int maxIterations = 40;
    
    // Latch will be decremented each time a thread exits.
    public static CountDownLatch latch = new CountDownLatch(2);

    // This class plays the role of the "Abstract Class" in the
    // Template Method pattern.
    public static abstract class BaseThread extends Thread
    {
        public BaseThread (int maxIterations,
                           String stringToPrint,
                           Semaphore pingSemaphore,
                           Semaphore pongSemaphore)
        {
            this.maxIterations = maxIterations;
            this.stringToPrint = stringToPrint;
            this.pingSemaphore = pingSemaphore;
            this.pongSemaphore = pongSemaphore;
        }

        // Abstract hook methods that determine the ping/pong protocol.
        abstract void acquire ();
        abstract void release ();

        // Main event loop that runs in a separate thread of control.
        // Note the use of the Template Method pattern's "template
        // method" here.
        public void run () 
        {
            try {
                for (int loopsDone = 1;; ++loopsDone)
                    {
                        acquire ();

                        System.out.println(stringToPrint + "(" + loopsDone + ")");

                        release ();

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
        protected Semaphore pingSemaphore;
        protected Semaphore pongSemaphore;
    }

    // Class that uses semaphores to implement the "ping" function.
    // This plays the role of the "Concrete Class" in the Template
    // Method pattern.
    public static class PingThread extends BaseThread
    {
        public PingThread (int maxIterations,
                           Semaphore ping,
                           Semaphore pong)
        {
            super (maxIterations, "Ping!", ping, pong);
        }

        // Hook method for "ping" acquire.
        void acquire ()
        {
            try 
                {
                    pingSemaphore.acquire ();
                } 
            catch (java.lang.InterruptedException e)
                {}
        }

        // Hook method for "ping" release.
        void release ()
        {
            pongSemaphore.release ();
        }

    }

    // Class that uses semaphores to implement the "pong" function.
    // This plays the role of the "Concrete Class" in the Template
    // Method pattern.
    public static class PongThread extends BaseThread
    {
        public PongThread (int maxIterations,
                           Semaphore ping,
                           Semaphore pong)
        {
            super (maxIterations, "Pong!", ping, pong);
        }
        
        // Hook method for "pong" acquire.
        void acquire ()
        {
            try 
                {
                    pongSemaphore.acquire ();
                } 
            catch (java.lang.InterruptedException e)
                {}
        }

        // Hook method for "pong" release.
        void release ()
        {
            pingSemaphore.release ();
        }

    }

    public static void main(String[] args) {
        try {         
            // Create the ping and pong semaphores that control
            // alternation between threads.
            Semaphore pingSemaphore = new Semaphore(1);
            Semaphore pongSemaphore = new Semaphore(0);

            System.out.println("Ready...Set...Go!");

            // Create the ping and pong threads.
            PingThread ping = new PingThread(maxIterations, pingSemaphore, pongSemaphore);
            PongThread pong = new PongThread(maxIterations, pingSemaphore, pongSemaphore);
            
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
