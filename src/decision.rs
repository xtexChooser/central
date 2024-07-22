pub enum Decision {
    Strike { id: String, fix: StrikeFix },
    TeeTee { id: String, fix: TeeTeeFix },
}

pub enum Kind {
    Strike,
    TeeTee,
}

pub fn find_decision<'a>(
    decisions: &'a [Decision],
    id: &str,
    kind: Kind,
) -> Option<&'a Decision> {
    for decision in decisions {
        match kind {
            Kind::Strike => {
                if let Decision::Strike { id: d_id, .. } = decision {
                    if d_id == id {
                        return Some(decision);
                    }
                }
            }
            Kind::TeeTee => {
                if let Decision::TeeTee { id: d_id, .. } = decision {
                    if d_id == id {
                        return Some(decision);
                    }
                }
            }
        }
    }
    None
}

#[derive(Clone, Copy)]
pub enum StrikeFix {
    S,
    Del,
}

#[derive(Clone, Copy)]
pub enum TeeTeeFix {
    Code,
    Kbd,
    Samp,
    Var,
    Mono,
}
