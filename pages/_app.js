import "@/styles/globals.css";
import { comicRelief, agbalumo, gochiHand } from "@/components/fonts";

export default function App({ Component, pageProps }) {
  return (
    <main className={`${comicRelief.variable} ${agbalumo.variable} ${gochiHand.variable}`}>
      <Component {...pageProps} />
    </main>
  );
}