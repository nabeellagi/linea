import CloudBackground from "@/components/CloudBackground";
import { H2 } from "@/components/Heading";
import { T1 } from "@/components/Title";

export default function Home() {
  return (
    <main className="relative isolate min-h-screen w-full overflow-clip">
      <CloudBackground
        maxClouds={12}
        minSpeed={8}
        maxSpeed={35}
        opacity={0.8}
      />
      <section className="relative z-10 flex min-h-screen w-full items-center justify-center px-4 py-16">
        <div className="flex flex-col items-center justify-center text-center">
          <T1 className="text-[#0785a8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)]">
            LINEA
          </T1>

          <H2 className="max-w-full text-balance text-[#0785a8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)]">
            Anything matrix. Solve it here!
          </H2>
        </div>
      </section>
    </main>
  );
}
