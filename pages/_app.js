import "katex/dist/katex.min.css";
import "@/styles/globals.css";
import { comicRelief, agbalumo, gochiHand } from "@/components/fonts";

export default function App({ Component, pageProps }) {
  return (
    <div className={`${comicRelief.variable} ${agbalumo.variable} ${gochiHand.variable}`}>
      <Component {...pageProps} />
    </div>
  );
}