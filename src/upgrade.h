
// Letrehoz egy elmaupg.res file-t, aminek elso negy byte-ja elma.res hossza.
// Veszi reg.res es upg.res file-ok 1.600.000-nel kezdodo 40.000 byte-jat es
// xor-ozza elma.res elso 40.000 byte-javal. Igy kapott eredmenyt beteszi
// elmaupg.res file tovabbi ket 40.000 byte-jaba (eloszor reg, utana upg),
// majd maradek elma.res file-t bemasolja elmaupg.res file-ba:
// Kulonallo exe-kent meghivni:
void makeupgradefile( void );

// Megnezi, hogy letezik-e elma.res file. Ha nem, megnezi hogy letezik-e
// elmaupg.res file. Ha nem hiba hogy nincsen elma.res. Ha igen akkor upgrade:
// Megnezi hogy van-e ervenyes (elso 8 byte) across.res file. Ha nincs, akkor
// hiba hogy upgrade-hez kell egy ilyen file. Megnezi hogy reg vagy upg es
// megfelelo 40.000 byte-tal xor-ozza across.res 1.600.000-nel kezdodo 40.000
// byte-jat es bemasolja elma.res elejere. Vegere meg elmaupg.res veget:
// Csak full verzioban van megvalositva:
void upgradehakell( void );

