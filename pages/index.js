import CloudBackground from "@/components/CloudBackground";
import { H2 } from "@/components/Heading";
import { T1, T2 } from "@/components/Title";
import WoodPanel from "@/components/WoodPanel";
import { useState } from "react";

export default function Home() {
  const [mode, setMode] = useState("Determinants")
  return (
    <main className="relative isolate h-screen overflow-hidden">
      <title>
        LINEA! ALL THINGS MATRIX
      </title>
      <div className="pointer-events-none fixed inset-0 -z-10">
        <CloudBackground
          maxClouds={12}
          minSpeed={8}
          maxSpeed={35}
          opacity={0.8}
        />
      </div>
      <div className="relative z-10 h-full overflow-y-auto snap-y snap-mandatory">
        <section className="flex min-h-screen snap-start snap-always items-center justify-center px-4 py-16">
          <div className="text-center">
            <T1 className="text-[#e2ffb8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)] [-webkit-text-stroke:2px_#0785a8]">
              LINEA
            </T1>
            <H2 className="text-[#0785a8] [text-shadow:0_4px_12px_rgba(0,0,0,0.4)]">
              Anything matrix. Solve it here!
            </H2>
          </div>
        </section>

        <section className="flex min-h-screen snap-start snap-always items-center justify-center px-4 py-16">
          <div className="text-center">
            <WoodPanel
              className="w-[80vw] min-w-[80vw] max-w-[80vw]"
              textureScale={2}
            >
              <T2>
                {mode}
              </T2>
            </WoodPanel>
          </div>
        </section>

      </div>
    </main>
  );
}