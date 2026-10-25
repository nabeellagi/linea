const paragraphStyles = {
  small     : "text-sm sm:text-base leading-relaxed",
  medium    : "text-base sm:text-lg leading-relaxed",
  large     : "text-lg sm:text-xl lg:text-2xl leading-relaxed",
};

export function P({
    size = "medium",
    children, 
    className = " ",
    ...props
}){
    const sizeClass = paragraphStyles[size] ?? paragraphStyles.medium;

    return(
        <p
        className={[
            "font-(--font-love-ya-like-a-sister)",
            "font-normal",
            "text-pretty",
            "max-w-prose",
            sizeClass,
            className,
        ].join(" ")}
        {...props}
        >
            {children}
        </p>
    )
}

export default P;