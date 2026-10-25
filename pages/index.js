import { H2 } from "@/components/Heading";
import { T1 } from "@/components/Title";

export default function Home() {
  return (
    <div className="bg-linear-to-b from-[#9deffc] to-[#3bd9f1] min-h-screen w-full flex items-center justify-center">
      <div className="flex flex-col items-center justify-center text-center">
        <T1 className="text-[#FFFADA] text-shadow-lg">LINEA</T1>
        <H2 className="text-[#FFFADA] text-shadow-lg">
          Anything matrix. Solve it here!
        </H2>
      </div>
    </div>
  );
}